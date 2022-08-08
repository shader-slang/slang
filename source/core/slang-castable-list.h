// slang-castable-list.h
#ifndef SLANG_CASTABLE_LIST_H
#define SLANG_CASTABLE_LIST_H

#include "../core/slang-basic.h"

#include "../core/slang-destroyable.h"

namespace Slang
{


/* A useful interface for handling lists of castable interfaces. Cannot hold nullptr */
class ICastableList : public ICastable
{
    SLANG_COM_INTERFACE(0x335f3d40, 0x934c, 0x40dc, { 0xb5, 0xe1, 0xf7, 0x6e, 0x40, 0x3, 0x62, 0x5 })

    typedef bool (*FindFunc)(ICastable* castable, void* data);

        /// Get the count of all interfaces held in the list
    virtual Count SLANG_MCALL getCount() = 0;
        /// Get the interface at the specified index
    virtual ICastable* SLANG_MCALL getAt(Index i) = 0;
        /// Add an item to the list
    virtual void SLANG_MCALL add(ICastable* unk) = 0;
        /// Add IUnknown, will cast to ICastable and if that's not possible will wrap
    virtual void SLANG_MCALL addUnknown(ISlangUnknown* unk) = 0;
        /// Remove item at index, remaining items stay in the same order
    virtual void SLANG_MCALL removeAt(Index i) = 0;
        /// Clear the list
    virtual void SLANG_MCALL clear() = 0;
        /// Find the first index of castable, or -1 if not found
    virtual Index SLANG_MCALL indexOf(ICastable* castable) = 0;
        /// Find the index interface (handling wrapping if necessary)
    virtual Index SLANG_MCALL indexOfUnknown(ISlangUnknown* unk) = 0;
        /// Find the first item that casts to non null
    virtual void* SLANG_MCALL find(const Guid& guid) = 0;
        /// Find the fast castable that matches the predicate
    virtual ICastable* SLANG_MCALL findWithPredicate(FindFunc func, void* data) = 0;
        /// Access the internal buffer (any mutation can invalidate this value)
    virtual ICastable*const* SLANG_MCALL getBuffer() = 0;
};

// Simply finding things in a ICastableList
template <typename T>
SLANG_FORCE_INLINE T* find(ICastableList* list)
{
    return reinterpret_cast<T*>(list->find(T::getTypeGuid())); 
}

template <typename T>
SLANG_FORCE_INLINE T* find(ICastableList* list, ICastableList::FindFunc func, void* data)
{
    return reinterpret_cast<T*>(list->findWithPredicate(T::getTypeGuid(), func, data));
}

/* Adapter interface to make a non castable types work as ICastable */
class IUnknownCastableAdapter : public ICastable
{
    SLANG_COM_INTERFACE(0x8b4aad81, 0x4934, 0x4a67, { 0xb2, 0xe2, 0xe9, 0x17, 0xfc, 0x29, 0x12, 0x54 });

    /// When using the adapter, this provides a way to directly get the internal no ICastable type
    virtual SLANG_NO_THROW ISlangUnknown* SLANG_MCALL getContained() = 0;
};

} // namespace Slang

#endif
