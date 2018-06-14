#ifndef SLANG_COM_PTR_H
#define SLANG_COM_PTR_H

#include "slang-defines.h"
#include "slang-result.h"

#include <assert.h>

namespace Slang {

/*! \brief ComPtr is a simple smart pointer that manages types which implement COM based interfaces.
\details A class that implements a COM, must derive from the IUnknown interface or a type that matches
it's layout exactly (such as IForwardUnknown). Trying to use this template with a class that doesn't follow
these rules, will lead to undefined behavior.
This is a 'strong' pointer type, and will AddRef when a non null pointer is set and Release when the pointer
leaves scope.
Using 'detach' allows a pointer to be removed from the management of the ComPtr.
To set the smart pointer to null, there is the method setNull, or alternatively just assign SLANG_NULL/nullptr.

One edge case using the template is that sometimes you want access as a pointer to a pointer. Sometimes this
is to write into the smart pointer, other times to pass as an array. To handle these different behaviors
there are the methods readRef and writeRef, which are used instead of the & (ref) operator. For example

\code

Void doSomething(ID3D12Resource** resources, IndexT numResources);

// ...
ComPtr<ID3D12Resource> resources[3];

doSomething(resources[0].readRef(), SLANG_COUNT_OF(resource));
\endcode

A more common scenario writing to the pointer

\code
IUnknown* unk = ...;

ComPtr<ID3D12Resource> resource;
Result res = unk->QueryInterface(resource.writeRef());
\endcode
*/

typedef SlangUUID Guid;

SLANG_FORCE_INLINE bool operator==(const Guid& aIn, const Guid& bIn)
{
    // Use the largest type the honors the alignment of Guid
    typedef uint32_t CmpType;
    union GuidCompare
    {
        Guid guid;
        CmpType data[sizeof(Guid) / sizeof(CmpType)];
    };
    // Type pun - so compiler can 'see' the pun and not break aliasing rules
    const CmpType* a = reinterpret_cast<const GuidCompare&>(aIn).data;
    const CmpType* b = reinterpret_cast<const GuidCompare&>(bIn).data;
    // Make the guid comparison a single branch, by not using short circuit
    return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2]) | (a[3] ^ b[3])) == 0;
}

SLANG_FORCE_INLINE bool operator!=(const Guid& a, const Guid& b)
{
    return !(a == b);
}

// Allows for defining of a GUID that works in C++ and C which defines in a format similar to microsofts INTERFACE style
// MIDL_INTERFACE("00000000-0000-0000-C000-00 00 00 00 00 46")

#define SLANG_GUID_BYTE(x, index) ((uint8_t)(SLANG_UINT64(0x##x) >> (8 * index)))

#define SLANG_MAKE_GUID(data0, data1, data2, shortTail, tail) \
	{ (uint32_t)(0x##data0), (uint16_t)(0x##data1), (uint16_t)(0x##data2), \
		{ (uint8_t)(0x##shortTail >> 8), (uint8_t)(0x##shortTail & 0xff), \
		SLANG_GUID_BYTE(tail,5), SLANG_GUID_BYTE(tail,4), SLANG_GUID_BYTE(tail,3), SLANG_GUID_BYTE(tail,2), SLANG_GUID_BYTE(tail,1), SLANG_GUID_BYTE(tail,0) \
	}}

// Compatible with Microsoft IUnknown
static const Guid IID_IComUnknown = SLANG_MAKE_GUID(00000000, 0000, 0000, C000, 000000000046);

typedef ISlangUnknown IComUnknown;

// Enum to force initializing as an attach (without adding a reference)
enum InitAttach
{
    INIT_ATTACH
};

template <class T>
class ComPtr
{
public:
	typedef T Type;
	typedef ComPtr ThisType;
	typedef IComUnknown* Ptr;

		/// Constructors
		/// Default Ctor. Sets to nullptr
	SLANG_FORCE_INLINE ComPtr() :m_ptr(nullptr) {}
		/// Sets, and ref counts.
	SLANG_FORCE_INLINE explicit ComPtr(T* ptr) :m_ptr(ptr) { if (ptr) ((Ptr)ptr)->addRef(); }
		/// The copy ctor
	SLANG_FORCE_INLINE ComPtr(const ThisType& rhs) : m_ptr(rhs.m_ptr) { if (m_ptr) ((Ptr)m_ptr)->addRef(); }

        /// Ctor without adding to ref count.
    SLANG_FORCE_INLINE explicit ComPtr(InitAttach, T* ptr) :m_ptr(ptr) { }
        /// Ctor without adding to ref count
    SLANG_FORCE_INLINE ComPtr(InitAttach, const ThisType& rhs) : m_ptr(rhs.m_ptr) { }

#ifdef SLANG_HAS_MOVE_SEMANTICS
		/// Move Ctor
	SLANG_FORCE_INLINE ComPtr(ThisType&& rhs) : m_ptr(rhs.m_ptr) { rhs.m_ptr = nullptr; }
		/// Move assign
	SLANG_FORCE_INLINE ComPtr& operator=(ThisType&& rhs) { T* swap = m_ptr; m_ptr = rhs.m_ptr; rhs.m_ptr = swap; return *this; }
#endif

	/// Destructor releases the pointer, assuming it is set
	SLANG_FORCE_INLINE ~ComPtr() { if (m_ptr) ((Ptr)m_ptr)->release(); }

	// !!! Operators !!!

	  /// Returns the dumb pointer
	SLANG_FORCE_INLINE operator T *() const { return m_ptr; }

	SLANG_FORCE_INLINE T& operator*() { return *m_ptr; }
		/// For making method invocations through the smart pointer work through the dumb pointer
	SLANG_FORCE_INLINE T* operator->() const { return m_ptr; }

		/// Assign
	SLANG_FORCE_INLINE const ThisType &operator=(const ThisType& rhs);
		/// Assign from dumb ptr
	SLANG_FORCE_INLINE T* operator=(T* in);

		/// Get the pointer and don't ref
	SLANG_FORCE_INLINE T* get() const { return m_ptr; }
		/// Release a contained nullptr pointer if set
	SLANG_FORCE_INLINE void setNull();

		/// Detach
	SLANG_FORCE_INLINE T* detach() { T* ptr = m_ptr; m_ptr = nullptr; return ptr; }
		/// Set to a pointer without changing the ref count
	SLANG_FORCE_INLINE void attach(T* in) { m_ptr = in; }

		/// Get ready for writing (nulls contents)
	SLANG_FORCE_INLINE T** writeRef() { setNull(); return &m_ptr; }
		/// Get for read access
	SLANG_FORCE_INLINE T*const* readRef() const { return &m_ptr; }

		/// Swap
	void swap(ThisType& rhs);

protected:
	/// Gets the address of the dumb pointer.
    // Disabled: use writeRef and readRef to get a reference based on usage.
	SLANG_FORCE_INLINE T** operator&();

	T* m_ptr;
};

//----------------------------------------------------------------------------
template <typename T>
void ComPtr<T>::setNull()
{
	if (m_ptr)
	{
		((Ptr)m_ptr)->release();
		m_ptr = nullptr;
	}
}
//----------------------------------------------------------------------------
/* template <typename T>
T** ComPtr<T>::operator&()
{
	assert(m_ptr == nullptr);
	return &m_ptr;
} */
//----------------------------------------------------------------------------
template <typename T>
const ComPtr<T>& ComPtr<T>::operator=(const ThisType& rhs)
{
	if (rhs.m_ptr) ((Ptr)rhs.m_ptr)->addRef();
	if (m_ptr) ((Ptr)m_ptr)->release();
	m_ptr = rhs.m_ptr;
	return *this;
}
//----------------------------------------------------------------------------
template <typename T>
T* ComPtr<T>::operator=(T* ptr)
{
	if (ptr) ((Ptr)ptr)->addRef();
	if (m_ptr) ((Ptr)m_ptr)->release();
	m_ptr = ptr;
	return m_ptr;
}
//----------------------------------------------------------------------------
template <typename T>
void ComPtr<T>::swap(ThisType& rhs)
{
	T* tmp = m_ptr;
	m_ptr = rhs.m_ptr;
	rhs.m_ptr = tmp;
}

} // namespace Slang

#endif // SLANG_COM_PTR_H
