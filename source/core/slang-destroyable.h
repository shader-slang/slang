#ifndef SLANG_CORE_DESTROYABLE_H
#define SLANG_CORE_DESTROYABLE_H

#include "slang-string.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

namespace Slang
{

/* An interface that allows for an object to implement 'destruction'. A destroyed 
interface/object should release any other contained references. 
Behavior of an interface that is IDestroyed should be defined on the interface. Typically 
it will produce an assert on debug builds. 
Calling destroy/isDestroyed can always be performed. */
class IDestroyable : public ICastable
{
    SLANG_COM_INTERFACE(0x99c6228e, 0xa82, 0x43eb, { 0x8f, 0xd1, 0xf3, 0x54, 0x3e, 0x2e, 0x86, 0xc0 } );

        /// Destroy. Can call on destroyed - is a no op.
    virtual SLANG_NO_THROW void SLANG_MCALL destroy() = 0;
        /// Once destroyed *no* functionality is supported other than IUnknown and destroy/isDestroyed
    virtual SLANG_NO_THROW bool SLANG_MCALL isDestroyed() = 0;
};

// Dynamic cast of ICastable derived types
template <typename T>
SLANG_FORCE_INLINE T* dynamicCast(ICastable* castable)
{
    if (castable)
    {
        void* obj = castable->castAs(T::getTypeGuid());
        return obj ? reinterpret_cast<T*>(obj) : ((T*)nullptr);
    }
    return nullptr;
}

// as style cast
template <typename T>
SLANG_FORCE_INLINE T* as(ICastable* castable)
{
    if (castable)
    {
        void* obj = castable->castAs(T::getTypeGuid());
        return obj ? reinterpret_cast<T*>(obj) : ((T*)nullptr);
    }
    return nullptr;
}

// A way to clone an interface (that derives from IClonable) such that it returns an interface 
// of the same type.
template <typename T>
SLANG_FORCE_INLINE ComPtr<T> cloneInterface(T* in)
{
    SLANG_ASSERT(in);
    // Must be derivable from clonable
    IClonable* clonable = in;
    // We can clone with the same interface
    T* clone = (T*)clonable->clone(T::getTypeGuid());
    // Clone must exist
    SLANG_ASSERT(clone);
    return ComPtr<T>(clone);
}

}

#endif // SLANG_CORE_DESTROYABLE_H
