#pragma once

class RingBuffer {
    // use as lock free queue between threads
private:
    RingBuffer() {}

public:
    RingBuffer(const char *name, uint32_t size)
            : m_name(name), m_nRIndex(0), m_nWIndex(0), m_nSize(size), m_arSlots(NULL) {
    }

    virtual ~RingBuffer() { if (m_arSlots) { delete[] m_arSlots; }}

    bool initialize() {
        m_arSlots = new void *[m_nSize + 1]; // 1 extra slow intentionally as workspace
        return m_arSlots ? true : false;
    }

    bool isWritable() { return getWritableSlotCount() > 0; }

    bool isReadable() { return getReadableSlotCount() > 0; }

    bool write(void *p) {
        if (isWritable()) {
            m_arSlots[m_nWIndex] = p;
            m_nWIndex = (m_nWIndex < m_nSize) ? (m_nWIndex + 1) : 0;
            return true;
        } else {
            FUNCLOG (Info, "%s: queue full", m_name.c_str());
            return false;
        }
    }

    bool peek(void **p) {
        if (isReadable()) {
            *p = m_arSlots[m_nRIndex];
            return true;
        } else {
            return false;
        }
    }

    bool read(void **p) {
        if (isReadable()) {
            *p = m_arSlots[m_nRIndex];
            m_nRIndex = (m_nRIndex < m_nSize) ? (m_nRIndex + 1) : 0;
            return true;
        } else {
            return false;
        }
    }

private:
    uint32_t getWritableSlotCount() {
        return (m_nWIndex < m_nRIndex) ?
               (m_nRIndex - m_nWIndex - 1) :
               (m_nSize - m_nWIndex + m_nRIndex);
    }

    uint32_t getReadableSlotCount() { return (m_nSize - getWritableSlotCount()); }

private:
    std::string m_name;
    volatile uint32_t m_nRIndex;
    volatile uint32_t m_nWIndex;
    volatile uint32_t m_nSize;
    void **m_arSlots;
};
