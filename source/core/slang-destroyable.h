#ifndef SLANG_CORE_DESTROYABLE_H
#define SLANG_CORE_DESTROYABLE_H

#include "slang-string.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

namespace Slang
{

/* An interface to provide a mechanism to cast, that doesn't require ref counting 
and doesn't have to return a pointer to a ISlangUnknown derived class */
class ICastable : ISlangUnknown
{
    SLANG_COM_INTERFACE(0x87ede0e1, 0x4852, 0x44b0, { 0x8b, 0xf2, 0xcb, 0x31, 0x87, 0x4d, 0xe2, 0x39 } );

        /// Can be used to cast to interfaces without reference counting. 
        /// Also provides access to internal implementations, when they provide a guid
        /// Can simulate a 'generated' interface as long as kept in scope by cast from. 
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) = 0;
};

/* An interface that allows for an object to implement 'destruction'. A destroyed 
interface/object should release any other contained references. 
Behavior of an interface that is IDestroyed should be defined on the interface. Typically 
it will produce an assert on debug builds. 
Calling destroy/isDestroyed can always be performed. */
class IDestroyable : ICastable
{
    SLANG_COM_INTERFACE(0x99c6228e, 0xa82, 0x43eb, { 0x8f, 0xd1, 0xf3, 0x54, 0x3e, 0x2e, 0x86, 0xc0 } );

        /// Destroy. Can call on destroyed - is a no op.
    virtual SLANG_NO_THROW void SLANG_MCALL destroy() = 0;
        /// Once destroyed *no* functionality is supported other than IUnknown and destroy/isDestroyed
    virtual SLANG_NO_THROW bool SLANG_MCALL isDestroyed() = 0;
};

}

#endif // SLANG_CORE_DESTROYABLE_H
