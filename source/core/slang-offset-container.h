// slang-offset-container.h
#ifndef SLANG_OFFSET_CONTAINER_H_INCLUDED
#define SLANG_OFFSET_CONTAINER_H_INCLUDED

#include "slang-basic.h"

namespace Slang {

enum
{
    kNull32Offset = 0,                                  
    kStartOffset = uint32_t(sizeof(uint64_t)),          ///< The offset to the first contained thing
};

template <typename T>
class Offset32Ref;

template <typename T>
class Offset32Ptr
{
public:
    typedef Offset32Ptr ThisType;

    const ThisType& operator=(const ThisType& rhs) { m_offset = rhs.m_offset; return *this; }
    bool operator==(const ThisType& rhs) const { return m_offset == rhs.m_offset; }
    bool operator!=(const ThisType& rhs) const { return m_offset != rhs.m_offset; }

    bool operator<(const ThisType& rhs) const { return m_offset < rhs.m_offset; }
    bool operator<=(const ThisType& rhs) const { return m_offset <= rhs.m_offset; }
    bool operator>(const ThisType& rhs) const { return m_offset > rhs.m_offset; }
    bool operator>=(const ThisType& rhs) const { return m_offset >= rhs.m_offset; }

    Offset32Ref<T> operator*();

    ThisType& operator++() { m_offset += uint32_t(sizeof(T)); return *this; }
    ThisType operator++(int) { const auto offset = m_offset; m_offset += uint32_t(sizeof(T)); return ThisType(offset); }

    ThisType& operator--() { m_offset -= sizeof(T); return *this; }
    ThisType operator--(int) { const auto offset = m_offset; m_offset -= uint32_t(sizeof(T)); return ThisType(offset); }

    friend ThisType operator+(const ThisType& a, int b) { return ThisType(a.m_offset + uint32_t(sizeof(T) * b)); }
    friend ThisType operator+(int a, const ThisType& b) { return ThisType(b.m_offset + uint32_t(sizeof(T) * a)); }

    friend ThisType operator+(const ThisType& a, uint32_t b) { return ThisType(a.m_offset + uint32_t(sizeof(T) * b)); }
    friend ThisType operator+(uint32_t a, const ThisType& b) { return ThisType(b.m_offset + uint32_t(sizeof(T) * a)); }

    friend ThisType operator+(const ThisType& a, Index b) { return ThisType(a.m_offset + uint32_t(sizeof(T) * b)); }
    friend ThisType operator+(Index a, const ThisType& b) { return ThisType(b.m_offset + uint32_t(sizeof(T) * a)); }

    void setNull() { m_offset = kNull32Offset; }
    Offset32Ptr():m_offset(kNull32Offset) {}
    Offset32Ptr(const ThisType& rhs): m_offset(rhs.m_offset) {}
    explicit Offset32Ptr(uint32_t offset): m_offset(offset) {}

    uint32_t m_offset;
};

template <typename T>
class Offset32Ref
{
public:
    typedef Offset32Ref ThisType;

    const ThisType& operator=(const ThisType& rhs) { m_offset = rhs.m_offset; return *this; }

    Offset32Ptr<T> operator&() { return Offset32Ptr<T>(m_offset); }

    Offset32Ref(const ThisType& rhs) : m_offset(rhs.m_offset) {}
    explicit Offset32Ref(uint32_t offset) : m_offset(offset) { SLANG_ASSERT(offset != kNull32Offset); }

    uint32_t m_offset;
};

// ---------------------------------------------------------------------------
template <typename T>
Offset32Ref<T> Offset32Ptr<T>::operator*()
{
    return Offset32Ref<T>(m_offset);
}

struct OffsetBase
{
    template <typename T>
    T* asRaw(const Offset32Ptr<T>& ptr) { return (T*)_getRaw(ptr.m_offset); }
    template <typename T>
    T& asRaw(const Offset32Ref<T>& ref) { return *(T*)_getRaw(ref.m_offset); }

    template <typename T>
    Offset32Ptr<T> asPtr(T* ptr) { return Offset32Ptr<T>(getOffset(ptr)); }
        /// Note the use of ptr when setting up a reference here - it's needed because a ref does not have to be backed by a pointer.
        /// And commonly is not when the const& and the thing referenced can be held in a word.
    template <typename T>
    Offset32Ref<T> asRef(T* ptr) { return Offset32Ref<T>(getOffset(ptr)); }

    uint8_t* _getRaw(uint32_t offset) { return (offset == kNull32Offset) ? nullptr : (m_data + offset); }
    uint32_t getOffset(const void* ptr)
    {
        if (ptr == nullptr)
        {
            return kNull32Offset;
        }
        ptrdiff_t diff = ((const uint8_t*)ptr) - m_data;
        SLANG_ASSERT(diff > 0 && size_t(diff) < m_dataSize);
        return uint32_t(diff);
    }

    OffsetBase():
        m_data(nullptr),
        m_dataSize(0)
    {
    }

    uint8_t* m_data;
    size_t m_dataSize;
};

template <typename T>
class Offset32Array
{
public:
    Offset32Ptr<const T> begin() const { return m_data; }
    Offset32Ptr<const T> end() const { return begin() + m_count; }

    Offset32Ptr<T> begin() { return m_data; }
    Offset32Ptr<T> end() { return begin() + m_count; }

    Index getCount() const { return Index(m_count); }
    
    Offset32Ref<const T> operator[](Index i) const { SLANG_ASSERT(i >= 0 && uint32_t(i) < m_count); return Offset32Ref<const T>((m_data + i).m_offset); }
    Offset32Ref<T> operator[](Index i) { SLANG_ASSERT(i >= 0 && uint32_t(i) < m_count); return Offset32Ref<T>((m_data + i).m_offset); }

    Offset32Array(Offset32Ptr<T> data, uint32_t count):m_data(data), m_count(count) {}

    Offset32Array():m_count(0) {}

    Offset32Ptr<T> m_data;
    uint32_t m_count;
};

struct OffsetString
{
    enum
    {
        kSizeBase = 251,
        kMaxSizeEncodeSize = 5,
    };

        /// Get contents as a slice
    UnownedStringSlice getSlice() const;
        /// Get null terminated string
    const char* getCstr() const;

        /// Decode the size. Returns the start of the string text, and outSize holds the size (NOT including terminating 0)
    static const char* decodeSize(const char* in, size_t& outSize);

        /// Returns the amount of bytes used, end encoding in 'encode'
    static size_t calcEncodedSize(size_t size, uint8_t encode[kMaxSizeEncodeSize]);
        /// Calculate the total size needed to store the string *including* terminating 0
    static size_t calcAllocationSize(const UnownedStringSlice& slice);

        /// Calculate the total size needed to store string. Size should be passed *without* terminating 0
    static size_t calcAllocationSize(size_t size);

    char m_sizeThenContents[1];
};

class OffsetContainer
{
public:

    template <typename T>
    Offset32Ptr<T> allocate()
    {
        void* data = allocate(sizeof(T), SLANG_ALIGN_OF(T));
        new (data) T();
        return Offset32Ptr<T>(m_base.getOffset(data));
    }

    template <typename T>
    Offset32Array<T> allocateArray(size_t size)
    {
        if (size == 0)
        {
            return Offset32Array<T>();
        }
        T* data = (T*)allocate(sizeof(T) * size, SLANG_ALIGN_OF(T));
        for (size_t i = 0; i < size; ++i)
        {
            new (data + i) T();
        }
        return Offset32Array<T>(Offset32Ptr<T>(m_base.getOffset(data)), uint32_t(size));
    }

        /// Get the base - which is needed for turning offsets into things
    OffsetBase* getBase() { return &m_base; }

    /// Allocate without alignment (effectively 1)
    void* allocate(size_t size);
    void* allocate(size_t size, size_t alignment);
    void* allocateAndZero(size_t size, size_t alignment);

    void fixAlignment(size_t alignment);

    Offset32Ptr<OffsetString> newString(const UnownedStringSlice& slice);
    Offset32Ptr<OffsetString> newString(const char* contents);

        /// Get the contained data
    uint8_t* getData() { return m_base.m_data; }
        /// Return the last used byte of the data
    size_t getDataCount() const { return m_base.m_dataSize; }

        /// Ctor
    OffsetContainer();
    ~OffsetContainer();

protected:
    OffsetBase m_base;
    size_t m_capacity;
};

} // namespace Slang

#endif
