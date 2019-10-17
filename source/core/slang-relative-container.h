// slang-relative-container.h
#ifndef SLANG_RELATIVE_CONTAINER_H_INCLUDED
#define SLANG_RELATIVE_CONTAINER_H_INCLUDED

#include "slang-basic.h"

namespace Slang {

enum
{
    kNullOffset32 = int32_t(0x80000000)
};

struct RelativeBase
{
    uint8_t* m_data;
};

template <typename T>
class Safe32Ptr
{
public:
    typedef Safe32Ptr ThisType;

    T& operator*() const { return *get(); }
    T* operator->() const { return get(); }
    operator T*() const { return get(); }

    Safe32Ptr(const ThisType& rhs) : m_offset(rhs.m_offset), m_base(rhs.m_base) {}

    const Safe32Ptr& operator=(const ThisType& rhs) { m_offset = rhs.m_offset; m_base = rhs.m_base; return *this; }

    SLANG_FORCE_INLINE T* Safe32Ptr<T>::get() const { return m_base ? ((T*)(m_base->m_data + m_offset)) : (T*)nullptr; }

    void setNull()
    {
        m_offset = kNullOffset32;
        m_base = nullptr;
    }

    Safe32Ptr() : m_base(nullptr), m_offset(kNullOffset32) {}

    Safe32Ptr(int32_t offset, RelativeBase* base) : m_offset(offset), m_base(base) {}

    RelativeBase* m_base;
    int32_t m_offset;
};

template <typename T>
class Relative32Ptr
{
public:
    typedef Relative32Ptr ThisType;

    T& operator*() const { return *get(); }
    T* operator->() const { return get(); }
    operator T*() const { return get(); }

    T* get()
    {
        uint8_t* nonConstThis = (uint8_t*)this;
        return (m_offset == kNullOffset32) ? nullptr : (T*)(nonConstThis + m_offset);
    }
    T* get() const
    {
        uint8_t* nonConstThis = const_cast<uint8_t*>((const uint8_t*)this);
        return (m_offset == kNullOffset32) ? nullptr : (T*)(nonConstThis + m_offset);
    }

    T* detach() { T* ptr = get(); m_offset = kNullOffset32; }

    void setNull() { m_offset = kNullOffset32; }

    SLANG_FORCE_INLINE void set(T* ptr) { m_offset = ptr ? int32_t(((uint8_t*)ptr) - ((const uint8_t*)this)) : uint32_t(kNullOffset32); }

    Relative32Ptr(const Safe32Ptr<T>& rhs) { set(rhs.get()); }
    Relative32Ptr(const ThisType& rhs) { set(rhs.get()); }

    Relative32Ptr() :m_offset(kNullOffset32) {}
    Relative32Ptr(T* ptr) { set(ptr); }

    const Relative32Ptr& operator=(const ThisType& rhs) { set(rhs.get()); return *this; }
    const Relative32Ptr& operator=(const Safe32Ptr<T>& rhs) { set(rhs.get()); return *this; }

    int32_t m_offset;
};

template <typename T>
class Safe32Array
{
public:

    Index getCount() const { return Index(m_count); }
    T* getData() { return m_data.get(); }
    const T* getData() const { return m_data.get(); }

    const T& operator[](Index i) const { SLANG_ASSERT(i >= 0 && i < m_count); return m_data.get()[i]; }
    T& operator[](Index i) { SLANG_ASSERT(i >= 0 && i < m_count); return m_data.get()[i]; }

    Safe32Array(Safe32Ptr<T> data, uint32_t count):m_data(data), m_count(count) {}

    Safe32Array():m_count(0) {}

    Safe32Ptr<T> m_data;
    uint32_t m_count;
};


template <typename T>
class Relative32Array
{
public:
    typedef Relative32Array ThisType;

    Index getCount() const { return Index(m_count); }
    T* getData() { return m_data.get(); }
    const T* getData() const { return m_data.get(); }

    const T& operator[](Index i) const { SLANG_ASSERT(i >= 0 && i < m_count); return m_data.get()[i]; }
    T& operator[](Index i) { SLANG_ASSERT(i >= 0 && i < m_count); return m_data.get()[i]; }

    Relative32Array(const Safe32Array<T>& rhs):
        m_count(rhs.m_count),
        m_data(rhs.m_data)
    {
    }

    Relative32Array() : m_count(0) {}
    Relative32Array(const ThisType& rhs) : m_count(rhs.m_count), m_data(rhs.m_data) {}

    uint32_t m_count;               ///< the size of the data
    Relative32Ptr<T> m_data;           ///< The data
};

struct RelativeString
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

class RelativeContainer
{
public:

    template <typename T>
    Safe32Ptr<T> allocate()
    {
        void* data = allocate(sizeof(T), SLANG_ALIGN_OF(T));
        new (data) T();
        return Safe32Ptr<T>(getOffset(data), &m_base);
    }

    template <typename T>
    Safe32Array<T> allocateArray(size_t size)
    {
        if (size == 0)
        {
            return Safe32Array<T>();
        }
        T* data = (T*)allocate(sizeof(T) * size, SLANG_ALIGN_OF(T));
        for (size_t i = 0; i < size; ++i)
        {
            new (data + i) T();
        }
        return Safe32Array<T>(Safe32Ptr<T>(getOffset(data), &m_base), uint32_t(size));
    }

    /// Allocate without alignment (effectively 1)
    void* allocate(size_t size);
    void* allocate(size_t size, size_t alignment);
    void* allocateAndZero(size_t size, size_t alignment);

    template <typename T>
    Safe32Ptr<T> toSafe(T* ptr) { SafePtr<T> safePtr; relPtr.m_offset = getOffset(); }
    int32_t getOffset(const void* ptr)
    {
        ptrdiff_t offset = ((const uint8_t*)ptr) - m_base.m_data; 
        if (offset < 0 || size_t(offset) > m_current)
        {
            return int32_t(kNullOffset32);
        }
        return int32_t(offset);
    }

    Safe32Ptr<RelativeString> newString(const UnownedStringSlice& slice);
    Safe32Ptr<RelativeString> newString(const char* contents);

        /// Get the contained data
    uint8_t* getData() { return m_base.m_data; }
        /// Return the last used byte of the data
    size_t getDataCount() const { return m_current; }

        /// Set the contents
    void set(void* data, size_t size);

    RelativeBase* getBase() { return &m_base; }

        /// Ctor
    RelativeContainer();
    ~RelativeContainer();


protected:
    size_t m_current;
    size_t m_capacity;
    RelativeBase m_base;
};


} // namespace Slang

#endif
