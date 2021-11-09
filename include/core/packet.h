#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include <map>
#include <vector>
#include <set>


#if defined(__i386__) || defined(__x86_64__)
#define XHTONS(x) x
#define XHTONL(x) x
#define XHTONLL(x) x
#define XNTOHS(x) x
#define XNTOHL(x) x
#define XNTOHLL(x) x
#else /* big end */
// don't not support big end
#endif /* __i386__ */

struct Varstr {
    const char *m_data;
    size_t m_size;

    Varstr(const char *data = NULL, size_t size = 0) {
        set(data, size);
    }

    void set(const char *data, size_t size) {
        m_data = data;
        m_size = size;
    }

    bool empty() const {
        return m_size == 0;
    }

    const char *data() const {
        return m_data;
    }

    size_t size() const {
        return m_size;
    }
};

struct PacketError : public std::runtime_error {
    PacketError(const std::string &w) :
            std::runtime_error(w) {
    }
};

struct PackError : public PacketError {
    PackError(const std::string &w) :
            PacketError(w) {
    }
};

struct UnpackError : public PacketError {
    UnpackError(const std::string &w) :
            PacketError(w) {
    }
};

class Pack;
class Unpack;

// pack message must be Marshallable
struct Marshallable {
    virtual void marshal(Pack &) const = 0;

    virtual void unmarshal(const Unpack &) = 0;

    virtual ~Marshallable() {}

    virtual std::ostream &trace(std::ostream &os) const {
        return os << "trace Marshallable [ not immplement ]";
    }
};

inline std::ostream &operator<<(std::ostream &os, const Marshallable &m) {
    return m.trace(os);
}

typedef uint32_t URI_TYPE;
#define HEADER_SIZE 10

struct Header {
    uint32_t length;
    uint32_t uri;
    uint16_t resCode;

    Header() : length(0), uri(0), resCode(200) {}

    void getRawHeader(void *buffer) {
        char *p = (char *) buffer;
        *(uint32_t *) p = XHTONL(length);
        p += sizeof(uint32_t);
        *(uint32_t *) p = XHTONL(uri);
        p += sizeof(uint32_t);
        *(uint16_t *) p = XHTONS(resCode);
    }

    void parseRawHeader(const void *buffer) {
        const char *p = (const char *) buffer;
        length = XNTOHL(*(const uint32_t *) p);
        p += sizeof(uint32_t);
        uri = XNTOHL(*(const uint32_t *) p);
        p += sizeof(uint32_t);
        resCode = XNTOHL(*(const uint16_t *) p);
    }
};

class PackBuffer {
private:
    char *buf;
    size_t len;
    size_t cap;

    PackBuffer(const PackBuffer &o);

    PackBuffer &operator=(const PackBuffer &o);

public:
    PackBuffer(size_t n = 256) {
        buf = (char *) malloc(n);
        len = 0;
        cap = n;
    }

    ~PackBuffer() {
        if (buf != NULL)
            free(buf);
    }

    // release memory must free by user
    char *release() {
        char *b = buf;
        buf = NULL;
        len = 0;
        cap = 0;
        return b;
    }

    const char *data() const {
        return buf;
    }

    size_t size() const {
        return len;
    }

    void append(const char *data, size_t size) {
        if (len + size > cap) {
            size_t n = std::max(len + size, cap * 2);
            reserve(n);
        }
        memcpy(buf + len, data, size);
        len += size;
    }

    void seek(size_t pos) {
        if (pos > cap)
            throw PackError("seek buffer overflow");
        len = pos;
    }

    void replace(size_t pos, const char *rep, size_t n) {
        if (pos + n > len)
            throw PackError("replace buffer overflow");
        memcpy(buf, rep, n);
    }

    void reserve(size_t n) {
        if (n <= cap)
            return;
        buf = (char *) realloc(buf, n);
        cap = n;
    }

    void clear() {
        len = 0;
    }
};

class Pack {
private:
    Pack(const Pack &o);

    Pack &operator=(const Pack &o);

    uint16_t xhtons(uint16_t i16) const {
        return XHTONS(i16);
    }

    uint32_t xhtonl(uint32_t i32) const {
        return XHTONL(i32);
    }

    uint64_t xhtonll(uint64_t i64) const {
        return XHTONLL(i64);
    }

    PackBuffer m_buffer;

public:
    Pack() {}

    Pack(URI_TYPE u, const Marshallable &m) {
        marshall(u, m);
    }

    void marshall(URI_TYPE u, const Marshallable &m) {
        m_buffer.seek(HEADER_SIZE);
        m.marshal(*this);

        Header header;
        header.uri = u;
        header.length = m_buffer.size();
        char buffer[HEADER_SIZE];
        header.getRawHeader(buffer);
        m_buffer.replace(0, buffer, HEADER_SIZE);
    }

    const char *data() const {
        return m_buffer.data();
    }

    size_t size() const {
        return m_buffer.size();
    }

    const char *body_data() const {
        return m_buffer.data() + HEADER_SIZE;
    }

    size_t body_size() const {
        return m_buffer.size() - HEADER_SIZE;
    }

    void clear() {
        return m_buffer.clear();
    }

    Pack &push(const void *s, size_t n) {
        m_buffer.append((const char *) s, n);
        return *this;
    }

    Pack &push_uint8(uint8_t u8) {
        return push(&u8, 1);
    }

    Pack &push_uint16(uint16_t u16) {
        u16 = xhtons(u16);
        return push(&u16, 2);
    }

    Pack &push_uint32(uint32_t u32) {
        u32 = xhtonl(u32);
        return push(&u32, 4);
    }

    Pack &push_uint64(uint64_t u64) {
        u64 = xhtonll(u64);
        return push(&u64, 8);
    }

    Pack &push_varstr(const Varstr &vs) {
        return push_varstr(vs.data(), vs.size());
    }

    Pack &push_varstr(const char *s) {
        return push_varstr(s, strlen(s));
    }

    Pack &push_varstr(const std::string &s) {
        return push_varstr(s.data(), s.size());
    }

    Pack &push_varstr(const void *s, size_t len) {
        if (len > 0xFFFF) throw PackError("push_varstr: varstr too big");
        return push_uint16(uint16_t(len)).push(s, len);
    }

    Pack &push_varstr32(const std::string &s) {
        return push_varstr32(s.data(), s.size());
    }

    Pack &push_varstr32(const void *s, size_t len) {
        if (len > 0xFFFFFFFF) throw PackError("push_varstr32: varstr too big");
        return push_uint32(uint32_t(len)).push(s, len);
    }
};

class Unpack {
private:
    Unpack(const Unpack &o);

    Unpack &operator=(const Unpack &o);

    uint16_t xntohs(uint16_t i16) const {
        return XNTOHS(i16);
    }

    uint32_t xntohl(uint32_t i32) const {
        return XNTOHL(i32);
    }

    uint64_t xntohll(uint64_t i64) const {
        return XNTOHLL(i64);
    }

public:
    Unpack(const void *data, size_t size) {
        m_data = (const char *) data;
        m_size = size;
    }

    virtual ~Unpack() {
        m_data = NULL;
    }

    std::string pop_varstr() const {
        Varstr vs = pop_varstr_ptr();
        return std::string(vs.data(), vs.size());
    }

    std::string pop_varstr32() const {
        Varstr vs = pop_varstr32_ptr();
        return std::string(vs.data(), vs.size());
    }

    std::string pop_fetch(size_t k) const {
        return std::string(pop_fetch_ptr(k), k);
    }

    void finish() const {
        if (!empty()) throw UnpackError("finish: too much data");
    }

    uint8_t pop_uint8() const {
        if (m_size < 1u)
            throw UnpackError("pop_uint8: not enough data");

        uint8_t i8 = *((uint8_t *) m_data);
        m_data += 1u;
        m_size -= 1u;
        return i8;
    }

    uint16_t pop_uint16() const {
        if (m_size < 2u)
            throw UnpackError("pop_uint16: not enough data");

        uint16_t i16 = *((uint16_t *) m_data);
        i16 = xntohs(i16);

        m_data += 2u;
        m_size -= 2u;
        return i16;
    }

    uint32_t pop_uint32() const {
        if (m_size < 4u)
            throw UnpackError("pop_uint32: not enough data");
        uint32_t i32 = *((uint32_t *) m_data);
        i32 = xntohl(i32);
        m_data += 4u;
        m_size -= 4u;
        return i32;
    }

    uint32_t peek_uint32() const {
        if (m_size < 4u)
            throw UnpackError("peek_uint32: not enough data");
        uint32_t i32 = *((uint32_t *) m_data);
        i32 = xntohl(i32);
        return i32;
    }

    uint64_t pop_uint64() const {
        if (m_size < 8u)
            throw UnpackError("pop_uint64: not enough data");
        uint64_t i64 = *((uint64_t *) m_data);
        i64 = xntohll(i64);
        m_data += 8u;
        m_size -= 8u;
        return i64;
    }

    Varstr pop_varstr_ptr() const {
        // Varstr { uint16_t size; const char * data; }
        Varstr vs;
        vs.m_size = pop_uint16();
        vs.m_data = pop_fetch_ptr(vs.m_size);
        return vs;
    }

    Varstr pop_varstr32_ptr() const {
        Varstr vs;
        vs.m_size = pop_uint32();
        vs.m_data = pop_fetch_ptr(vs.m_size);
        return vs;
    }

    const char *pop_fetch_ptr(size_t k) const {
        if (m_size < k) {
            //abort();
            throw UnpackError("pop_fetch_ptr: not enough data");
        }

        const char *p = m_data;
        m_data += k;
        m_size -= k;
        return p;
    }

    bool header(Header &h) {
        if (m_size < HEADER_SIZE)
            return false;
        h.parseRawHeader(m_data);
        return true;
    }

    void pop_header() {
        if (m_size >= HEADER_SIZE) {
            m_data += HEADER_SIZE;
            m_size -= HEADER_SIZE;
        }
    }

    bool empty() const {
        return m_size == 0;
    }

    const char *data() const {
        return m_data;
    }

    size_t size() const {
        return m_size;
    }

private:
    mutable const char *m_data;
    mutable size_t m_size;
};

inline Pack &operator<<(Pack &p, const Marshallable &m) {
    m.marshal(p);
    return p;
}

inline const Unpack &operator>>(const Unpack &p, const Marshallable &m) {
    const_cast<Marshallable &>(m).unmarshal(p);
    return p;
}

// base type helper
inline Pack &operator<<(Pack &p, bool sign) {
    p.push_uint8(sign ? 1 : 0);
    return p;
}

// base type helper
inline Pack &operator<<(Pack &p, uint8_t i8) {
    p.push_uint8(i8);
    return p;
}

inline Pack &operator<<(Pack &p, uint16_t i16) {
    p.push_uint16(i16);
    return p;
}

inline Pack &operator<<(Pack &p, uint32_t i32) {
    p.push_uint32(i32);
    return p;
}

inline Pack &operator<<(Pack &p, uint64_t i64) {
    p.push_uint64(i64);
    return p;
}

inline Pack &operator<<(Pack &p, const std::string &str) {
    p.push_varstr(str);
    return p;
}

inline Pack &operator<<(Pack &p, const Varstr &pstr) {
    p.push_varstr(pstr);
    return p;
}

inline const Unpack &operator>>(const Unpack &p, Varstr &pstr) {
    pstr = p.pop_varstr_ptr();
    return p;
}

// pair.first helper
// XXX std::map::value_type::first_type unpack 需要特别定义
inline const Unpack &operator>>(const Unpack &p, uint32_t &i32) {
    i32 = p.pop_uint32();
    return p;
}

inline const Unpack &operator>>(const Unpack &p, uint64_t &i64) {
    i64 = p.pop_uint64();
    return p;
}

inline const Unpack &operator>>(const Unpack &p, std::string &str) {
    // XXX map::value_type::first_type
    str = p.pop_varstr();
    return p;
}

inline const Unpack &operator>>(const Unpack &p, uint16_t &i16) {
    i16 = p.pop_uint16();
    return p;
}

inline const Unpack &operator>>(const Unpack &p, uint8_t &i8) {
    i8 = p.pop_uint8();
    return p;
}

inline const Unpack &operator>>(const Unpack &p, bool &sign) {
    sign = (p.pop_uint8() == 0) ? false : true;
    return p;
}


template<class T1, class T2>
inline std::ostream &operator<<(std::ostream &s, const std::pair<T1, T2> &p) {
    s << p.first << '=' << p.second;
    return s;
}

template<class T1, class T2>
inline Pack &operator<<(Pack &s, const std::pair<T1, T2> &p) {
    s << p.first << p.second;
    return s;
}

template<class T1, class T2>
inline const Unpack &operator>>(const Unpack &s, std::pair<const T1, T2> &p) {
    const T1 &m = p.first;
    T1 &m2 = const_cast<T1 &>(m);
    s >> m2 >> p.second;
    return s;
}

template<class T1, class T2>
inline const Unpack &operator>>(const Unpack &s, std::pair<T1, T2> &p) {
    s >> p.first >> p.second;
    return s;
}

// container marshal helper
template<typename ContainerClass>
inline void marshal_container(Pack &p, const ContainerClass &c) {
    p.push_uint32(uint32_t(c.size())); // use uint32_t ...
    for (typename ContainerClass::const_iterator i = c.begin(); i != c.end(); ++i)
        p << *i;
}

template<typename OutputIterator>
inline void unmarshal_container(const Unpack &p, OutputIterator i) {
    for (uint32_t count = p.pop_uint32(); count > 0; --count) {
        typename OutputIterator::container_type::value_type tmp;
        p >> tmp;
        *i = tmp;
        ++i;
    }
}

template<typename ContainerClass>
inline std::ostream &trace_container(std::ostream &os, const ContainerClass &c, char div = '\n') {
    for (typename ContainerClass::const_iterator i = c.begin(); i != c.end(); ++i)
        os << *i << div;
    return os;
}

template<class K, class V>
inline Pack &operator<<(Pack &p, const std::map<K, V> &c) {
    p.push_uint32(uint32_t(c.size()));
    for (typename std::map<K, V>::const_iterator i = c.begin(); i != c.end(); ++i)
        p << *i;
    return p;
}

// 容器长度异常检测
template<class T>
inline void marshal_count_check(uint32_t count, uint32_t size) {
    size_t allsize = count * sizeof(T);
    if (allsize > 128 * 1024 * 1024) // 未知类型，长度大于128MB
        throw UnpackError("marshal count check unknown error");
}

// bool 类型封装协议为uint8_t
template<>
inline void marshal_count_check<bool>(uint32_t count, uint32_t size) {
    size_t allsize = count * sizeof(uint8_t);
    if (allsize > size)
        throw UnpackError("marshal count check bool error");
}

template<>
inline void marshal_count_check<uint8_t>(uint32_t count, uint32_t size) {
    size_t allsize = count * sizeof(uint8_t);
    if (allsize > size)
        throw UnpackError("marshal count check uint8_t error");
}

template<>
inline void marshal_count_check<uint16_t>(uint32_t count, uint32_t size) {
    size_t allsize = count * sizeof(uint16_t);
    if (allsize > size)
        throw UnpackError("marshal count check uint16_t error");
}

template<>
inline void marshal_count_check<uint32_t>(uint32_t count, uint32_t size) {
    size_t allsize = count * sizeof(uint32_t);
    if (allsize > size)
        throw UnpackError("marshal count check uint32_t error");
}

template<>
inline void marshal_count_check<uint64_t>(uint32_t count, uint32_t size) {
    size_t allsize = count * sizeof(uint64_t);
    if (allsize > size)
        throw UnpackError("marshal count check uint64_t error");
}

template<class K, class V>
inline const Unpack &operator>>(const Unpack &s, std::map<K, V> &c) {
    for (uint32_t count = s.pop_uint32(); count > 0; --count) {
        typename std::map<K, V>::value_type tmp;
        s >> tmp;
        c.insert(tmp);
    }
    return s;
}

template<class T>
inline Pack &operator<<(Pack &p, const std::vector<T> &c) {
    p.push_uint32(uint32_t(c.size()));
    for (typename std::vector<T>::const_iterator i = c.begin(); i != c.end(); ++i)
        p << *i;
    return p;
}

template<class T>
inline const Unpack &operator>>(const Unpack &s, std::vector<T> &c) {
    uint32_t count = s.pop_uint32();
    if (count == 0)
        return s;
    marshal_count_check<T>(count, s.size());
    c.resize(count);
    for (uint32_t i = 0; i < count; ++i) {
        s >> c[i];
    }
    return s;
}

template<class T>
inline Pack &operator<<(Pack &p, const std::set<T> &c) {
    p.push_uint32(uint32_t(c.size()));
    for (typename std::set<T>::const_iterator i = c.begin(); i != c.end(); ++i)
        p << *i;
    return p;
}

template<class T>
inline const Unpack &operator>>(const Unpack &s, std::set<T> &c) {
    for (uint32_t count = s.pop_uint32(); count > 0; --count) {
        typename std::set<T>::value_type tmp;
        s >> tmp;
        c.insert(tmp);
    }
    return s;
}
