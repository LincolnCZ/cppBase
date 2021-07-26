//StrBlob.h文件

#include <vector>
#include <string>
#include <initializer_list>
#include <memory>
#include <stdexcept>

using namespace std;

// forward declaration needed for friend declaration in StrBlob
class StrBlobPtr;

class StrBlob {
    friend class StrBlobPtr;

public:
    typedef std::vector<std::string>::size_type size_type;

    // constructors
    StrBlob() : data(std::make_shared<std::vector<std::string>>()) {}

    StrBlob(std::initializer_list<std::string> il);

    // size operations
    size_type size() const { return data->size(); }

    bool empty() const { return data->empty(); }

    // add and remove elements
    void push_back(const std::string &t) { data->push_back(t); }

    void pop_back();

    // element access
    string &front();

    const string &front() const;

    string &back();

    const string &back() const;

    // interface to StrBlobPtr
    StrBlobPtr begin();  // can't be defined until StrBlobPtr is
    StrBlobPtr end();

private:
    std::shared_ptr<std::vector<std::string>> data;

    // throws msg if data[i] isn't valid
    void check(size_type i, const std::string &msg) const;
};

// constructor
inline
StrBlob::StrBlob(std::initializer_list<std::string> il) :
        data(std::make_shared<std::vector<std::string>>(il)) {}

void StrBlob::check(size_type i, const std::string &msg) const {
    if (i >= data->size())
        throw std::out_of_range(msg);
}

//pop_back、front 和 back操作访问 vector 中的元素。这些操作在试图访问元素之前必须检查元素是否存在

string &StrBlob::front() {
    //如果vector为空，check 会抛出一个异常
    check(0, "front on empty StrBlob");
    return data->front();
}

const string &StrBlob::front() const {
    //如果vector为空，check 会抛出一个异常
    check(0, "front on empty StrBlob");
    return data->front();
}

string &StrBlob::back() {
    //如果vector为空，check 会抛出一个异常
    check(0, "back on empty StrBlob");
    return data->back();
}

const string &StrBlob::back() const {
    //如果vector为空，check 会抛出一个异常
    check(0, "back on empty StrBlob");
    return data->back();
}

void StrBlob::pop_back() {
    //如果vector为空，check 会抛出一个异常
    check(0, "pop_back on empty StrBlob");
    return data->pop_back();
}

//====================================================================================

// StrBlobPtr throws an exception on attempts to access a nonexistent element
class StrBlobPtr {
    friend bool eq(const StrBlobPtr &, const StrBlobPtr &);

public:
    StrBlobPtr() : curr(0) {}

    StrBlobPtr(StrBlob &a, size_t sz = 0) : wptr(a.data), curr(sz) {}

    std::string &deref() const;

    StrBlobPtr &incr();       // prefix version 前缀递增
    StrBlobPtr &decr();       // prefix version
private:
    // check returns a shared_ptr to the vector if the check succeeds
    std::shared_ptr<std::vector<std::string>>
    check(std::size_t, const std::string &) const;

    // store a weak_ptr, which means the underlying（底层） vector might be destroyed
    std::weak_ptr<std::vector<std::string>> wptr;
    std::size_t curr;      // current position within the array
};

inline
std::string &StrBlobPtr::deref() const {
    auto p = check(curr, "dereference past end");
    return (*p)[curr];  // (*p) is the vector to which this object points
}

//StrBlobPtr 指向的vector 可能已经被释放了。如果 vector 己销毁，lock 将返回 一个空指针。
inline
std::shared_ptr<std::vector<std::string>>
StrBlobPtr::check(std::size_t i, const std::string &msg) const {
    //由于对象可能不存在，我们不能使用 weak_ptr 直接访问对象，而必须调用 lock
    auto ret = wptr.lock();   // is the vector still around?
    if (!ret)
        throw std::runtime_error("unbound StrBlobPtr");

    if (i >= ret->size())
        throw std::out_of_range(msg);
    return ret; // otherwise, return a shared_ptr to the vector
}

// prefix: return a reference to the incremented object
inline
StrBlobPtr &StrBlobPtr::incr() {
    // if curr already points past the end of the container, can't increment it
    check(curr, "increment past end of StrBlobPtr");
    ++curr;       // advance the current state
    return *this;
}

inline
StrBlobPtr &StrBlobPtr::decr() {
    // if curr is zero, decrementing it will yield an invalid subscript
    --curr;       // move the current state back one element}
    check(-1, "decrement past begin of StrBlobPtr");
    return *this;
}

// begin and end members for StrBlob
inline
StrBlobPtr
StrBlob::begin() {
    return StrBlobPtr(*this);
}

inline
StrBlobPtr
StrBlob::end() {
    auto ret = StrBlobPtr(*this, data->size());
    return ret;
}

// named equality operators for StrBlobPtr
inline
bool eq(const StrBlobPtr &lhs, const StrBlobPtr &rhs) {
    auto l = lhs.wptr.lock(), r = rhs.wptr.lock();
    // if the underlying vector is the same
    if (l == r)
        // then they're equal if they're both null or
        // if they point to the same element
        return (!r || lhs.curr == rhs.curr);
    else
        return false; // if they point to difference vectors, they're not equal
}

inline
bool neq(const StrBlobPtr &lhs, const StrBlobPtr &rhs) {
    return !eq(lhs, rhs);
}

#include <iostream>

using std::cout; using std::endl;

int main() {
    StrBlob b1;
    {
        StrBlob b2 = {"a", "an", "the"};
        b1 = b2;
        b2.push_back("about");
        cout << b2.size() << endl;
    }
    cout << b1.size() << endl;

    for (auto it = b1.begin(); neq(it, b1.end()); it.incr())
        cout << it.deref() << endl;

    return 0;
}