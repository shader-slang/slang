// slang-relative-container.h
#ifndef SLANG_RELATIVE_CONTAINER_H_INCLUDED
#define SLANG_RELATIVE_CONTAINER_H_INCLUDED

#include "slang-basic.h"

namespace Slang {

/*
The purpose of RelativeContainer and related types is to provide a mechanism to easily serialize relative structures.

The root idea here is the "relative pointer". A typical pointer in a language like C/C++ holds the absolute address
of the thing that is being pointed to. This introduces a problem if the structure is serialized in that
it is highly likely that the structures will be placed at different addresses. This means that the absolute
pointers will not point to the correct places, and so not be usable when read back from a serialization, or
when moved to another location.

A relative pointer means a pointer that points to something relative
to the *location of the pointer*. The Relative32Ptr uses a 32 bit offset from the pointers location in memory. This
means such a pointer can address a 4Gb address space, but more realistically it gives it a 2Gb address space as this
is the size such that a pointer at any address can point to any other address. For specialized uses, it can be useful
to have 8 bit, 16 bit, 64 bit and scaled relative pointers. For the purposes here though 32 bits works well enough
for current use cases.

Special care is needed when using relative pointers - both when constructing structures that contain them, reading
them and in general usage.

For simplicity here we store all relative pointers within a single contiguous allocation. This allocation is
typically managed by the RelativeContainer. For simplicity it's easiest to claim that all relative pointers
*can only exist* in this address space. That is no relative pointer should be decared as a variable on the stack, and
similarly no struct, or other derived type holding a relative pointer should be held on the stack.

Why? Most simply in a 64bit address space there is no guarentee that say a 32bit relative pointer *can* point to the
memory in the RelativeContainer. For similar reasons relative ptrs cannot be held in the heap, or in a typical ADT
container (like std::vector). In summary RelativePtr can *only* be stored in contiguous chunk of memory designed for
the purpose - such as RelativeContainer, or a continuous chunk of memory that has been serialized in.

This presents a problem - in how do we create and use such structures? For reading the process is simple - in that we
can just turn the relative pointer into a regular raw pointer and use it. When creating structures - unless you know
the allocated space (in the RelativeContainer or some other piece of memory) is larger than required, then special care
is needed, because when a new larger piece of memory is allocated to hold everything, all of the absolute pointers
will likely be invalidated.

To work around this problem we have the Safe32Ptr. This is a pointer which holds a pointer relative to the start of the
allocation as well as knowing what the base allocation is. So if there is a change in the base allocation address -
for example when the RelativeContainer resizes the backing memory, the Safe32Ptr is aware of the change and everything continues to
work as expected. Safe32Ptr are much more like normal pointers - and they can be stored on the stack or in other structures
like say a vector without problems. On the other hand a Safe32Ptr *cannot* be stored within stuctures held within the RelativeContainer,
as it would no longer have the correct properties to be serializable (it would contain an absolute pointer - the one in the SafePtr).

We can divide types into to sets. 'Relative types' and 'Non relative types'. 'Relative types' are Relative pointers/arrays,
and value types (such as float, int etc), and POD types constructed from those types. Everything else is not a 'relative type'.
Any relative type that has a relative pointer (such as Relative32Ptr and Relative32Array) can only be allocated in a 'relative aware'
piece or memory such as RelativeContainer, or suitable contiguous piece of memory.

So in basic usage - Relative32Ptr can only be stored inside of RelativeContainer or part of contiguous memory arranged to
support them. Conversely Safe32Ptr can only be used outside of the places RelativePtrs can be used.
They can both point to the same things though. RelativePtrs can be thought of pointers for use in serialization, and SafePtrs
as pointers used to construct things using RelativePtrs.

With that out of the way there is one last caveat - and it is around use of Safe32Ptr. That the Safe32Ptr is safe to use
even if the underlying memory is moved. But raw pointers (which are of course absolute) from it are only valid whilst there
are no changes to the location of memory. That Relative32Ptr and Safe32Ptrs are convertible between each other.

For example 

```

struct Thing
{
    Relative32Ptr<RelativeString> text;
    int value;
};

void func()
{
    RelativeContainer container;

    // BAD! Can't construct anything containing a Relative32Ptr on the stack.
    Thing thing;

    // BAD! We are closer - thing is constructed in the container, but we cannot have Relative32Ptrs held on the stack.
    Relative32Ptr<Thing> thing = container.newObject<Thing>();

    // Ok - this is now correct
    Safe32Ptr<Thing> thing = container.newObject<Thing>();

    // We can write and read things via the Safe32Ptr
    thing->value = 10;
    const int value = thing->value;

    // Now lets write to it
    {
        // We can have raw pointer (or reference) to a thing but we need to be *careful* if we allocate 
        Thing* rawThing = thing;
        // We are okay here, nothing between getting the raw pointer and the write allocated/newed anything on the RelativeContainer
        rawThing->value = 20;

        // Lets set up name 
        Safe32Ptr<RelativeString> text = relativeContainer.newString("Hello World!");

        // BAD! Thr rawThing point could now be invalid because the call to newString may have had to allocate more memory
        rawThing->text = text;

        // This is okay because access is through the Safe32Ptr
        thing->text = text;

        // Or we can update rawThing such that is up to date
        rawThing = thing;
        // So now this is okay again
        rawThing->text = text;
    }
}

```

Safe32Array and Relative32Array have very similar behaviors to Safe32Ptr and Relative32Ptr and can be used in the same places
for the same purposes, but their use revolves around arrays. The arrays data is always allocated in the *RelativeContainer* so
the arrays contents *even with* Safe32Array, can only contain Relative types. 

For example

```

// BAD! The element types cannot contain any absolute pointers and that includes SafePtr
Safe32Array<Safe32Ptr<RelativeString>> array = container.newArray<Safe32Ptr<RelativeString>>(10);

// Ok
Safe32Array<Relative32Ptr<RelativeString>> array = container.newArray<Relative32<Ptr<RelativeString>>(10);
// I can now set array element in the normal way
array[1] = container.newString("Hello");
array[2] = container.newString("World!");
```

*/


/* A type that is used to hold the base address of the contiguous memory that holds either RelativePtr and related types
and/or is pointed to by Safe32Ptrs.

The g_null member is a special singleton version that just holds m_data as nullptr, allows the representation of 'nullptr' on
a Safe32Ptr to be that RelativeBase with an offset of 0.
*/
struct RelativeBase
{
    uint8_t* m_data;

    static RelativeBase g_null;
};

/* A pointer to items held in RelativeContainer that remains correct even if the memory inside RelativeContainer moves.
Safe32Ptr can be allocated on the stack, on the heap, used in containers such as List, std::vector.
*/
template <typename T>
class Safe32Ptr
{
public:
    typedef Safe32Ptr ThisType;

    T& operator*() const { return *get(); }
    T* operator->() const { return get(); }
    operator T*() const { return get(); }

    const Safe32Ptr& operator=(const ThisType& rhs) { m_offset = rhs.m_offset; m_base = rhs.m_base; return *this; }
    SLANG_FORCE_INLINE T* get() const { return (T*)(m_base->m_data + m_offset); } 

    void setNull()
    {
        m_offset = 0;
        m_base = &RelativeBase::g_null; 
    }

    Safe32Ptr(const ThisType& rhs) : m_offset(rhs.m_offset), m_base(rhs.m_base) {}
    Safe32Ptr() : m_base(&RelativeBase::g_null), m_offset(0) {}
    Safe32Ptr(uint32_t offset, RelativeBase* base) : m_offset(offset), m_base(base) {}

    RelativeBase* m_base;
    uint32_t m_offset;
};


enum
{
    kRelative32PtrNull = int32_t(0x80000000)
};

/* A 32 bit relative pointer. It can only be held in contiguous memory designed for it's usage (like RelativeContainer). The thing
that it points to is relative to the address *of the pointer*.

This means that in normal usage should *not* be allocated on the stack, on the heap (unless as part of contiguous piece of memory
designed for usage), or in a container such as std::vector or List.

That because pointers are relative, we use a special value `kRelative32PtrNull` to indicate a pointer is nullptr. 0 could not be used
because if we had

```
struct Thing
{
    RelativePtr<Thing> thingPtr;
    int someThingElse;
};
```

It might be valid for thingPtr to point to Thing and in that case it's offset would be 0, and this confused with nullptr if 0 was
used to represent nullptr.
*/
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
        return (m_offset == kRelative32PtrNull) ? nullptr : (T*)(nonConstThis + m_offset);
    }
    T* get() const
    {
        uint8_t* nonConstThis = const_cast<uint8_t*>((const uint8_t*)this);
        return (m_offset == kRelative32PtrNull) ? nullptr : (T*)(nonConstThis + m_offset);
    }

    T* detach() { T* ptr = get(); m_offset = kRelative32PtrNull; }

    void setNull() { m_offset = kRelative32PtrNull; }

    SLANG_FORCE_INLINE void set(T* ptr) { m_offset = ptr ? int32_t(((uint8_t*)ptr) - ((const uint8_t*)this)) : uint32_t(kRelative32PtrNull); }

    Relative32Ptr(const Safe32Ptr<T>& rhs) { set(rhs.get()); }
    Relative32Ptr(const ThisType& rhs) { set(rhs.get()); }

    Relative32Ptr() :m_offset(kRelative32PtrNull) {}
    Relative32Ptr(T* ptr) { set(ptr); }

    const Relative32Ptr& operator=(const ThisType& rhs) { set(rhs.get()); return *this; }
    const Relative32Ptr& operator=(const Safe32Ptr<T>& rhs) { set(rhs.get()); return *this; }

    int32_t m_offset;
};

/* Much like SafePtr this is an array but whose memory is stored inside the RelativeContainer. This means elements types 
must be 'relative types'. */
template <typename T>
class Safe32Array
{
public:
    const T* begin() const { return m_data; }
    const T* end() const { return begin() + m_count; }

    T* begin() { return m_data; }
    T* end() { return begin() + m_count; }

    Index getCount() const { return Index(m_count); }
    T* getData() { return m_data.get(); }
    const T* getData() const { return m_data.get(); }

    const T& operator[](Index i) const { SLANG_ASSERT(i >= 0 && uint32_t(i) < m_count); return m_data.get()[i]; }
    T& operator[](Index i) { SLANG_ASSERT(i >= 0 && uint32_t(i) < m_count); return m_data.get()[i]; }

    Safe32Array(Safe32Ptr<T> data, uint32_t count):m_data(data), m_count(count) {}

    Safe32Array():m_count(0) {}

    Safe32Ptr<T> m_data;
    uint32_t m_count;
};

/* Much like a RelativePtr this is an array whose elements are stored inside the RelativeContainer. This means element types can only be 'relative types'. */
template <typename T>
class Relative32Array
{
public:
    typedef Relative32Array ThisType;

    const T* begin() const { return m_data; }
    const T* end() const { return begin() + m_count; }

    T* begin() { return m_data; }
    T* end() { return begin() + m_count; }

    Index getCount() const { return Index(m_count); }
    T* getData() { return m_data.get(); }
    const T* getData() const { return m_data.get(); }

    const T& operator[](Index i) const { SLANG_ASSERT(i >= 0 && uint32_t(i) < m_count); return m_data.get()[i]; }
    T& operator[](Index i) { SLANG_ASSERT(i >= 0 && uint32_t(i) < m_count); return m_data.get()[i]; }

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

/** RelativeString is used for storing strings within a RelativeContainer. Strings are stored with the initial byte indicating the size
of the string. Note that all relative strings are stored with a terminating zero, and that the terminating zero is *NOT* included in
the encoded size. */
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

/* RelativeContainer is a type designed to manage the construction structures around 'relative types'. In particular it allows
for construction of relative structures where their total relative encoded size is not known at the outset.

The main mechanism to make this work is via the use of SafeXXX types, which when constructed from the RelativeContainer will
maintain valid values, even if the underlying backing memories location is changed. 
*/
class RelativeContainer
{
public:

    template <typename T>
    Safe32Ptr<T> newObject()
    {
        void* data = allocate(sizeof(T), SLANG_ALIGN_OF(T));
        new (data) T();
        return Safe32Ptr<T>(getOffset(data), &m_base);
    }

    template <typename T>
    Safe32Array<T> newArray(size_t size)
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

        /// Make a raw pointer into a safe ptr
    template <typename T>
    Safe32Ptr<T> toSafe(T* in)
    {
        Safe32Ptr<T> dst;
        if (in)
        {
            dst.m_base = &m_base;
            dst.m_offset = getOffset(in);
        }
        return dst;
    }

    /// Allocate without alignment (effectively 1)
    void* allocate(size_t size);
    void* allocate(size_t size, size_t alignment);
    void* allocateAndZero(size_t size, size_t alignment);

    void fixAlignment(size_t alignment);

    SLANG_FORCE_INLINE uint32_t getOffset(const void* ptr) const
    {
        ptrdiff_t offset = ((const uint8_t*)ptr) - m_base.m_data; 
        SLANG_ASSERT(offset >= 0 && size_t(offset) < m_current);
        return uint32_t(offset);
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
