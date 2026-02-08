#ifndef SLANG_PROXY_SHARED_LIBRARY_H
#define SLANG_PROXY_SHARED_LIBRARY_H

#include "proxy-base.h"
#include "proxy-macros.h"

namespace SlangRecord
{

class SharedLibraryProxy : public ProxyBase<ISlangSharedLibrary>
{
public:
    SLANG_COM_INTERFACE(
        0x7f8e1a6d,
        0x4b5c,
        0x7d2e,
        {0xbf, 0xa0, 0x9b, 0x6c, 0x5d, 0x4e, 0x3f, 0xc0})

    explicit SharedLibraryProxy(ISlangSharedLibrary* actual)
        : ProxyBase(actual)
    {
    }

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(SharedLibraryProxy)

    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject)
        SLANG_OVERRIDE
    {
        if (!outObject)
            return SLANG_E_INVALID_ARG;

        if (uuid == SharedLibraryProxy::getTypeGuid() || uuid == ISlangSharedLibrary::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangSharedLibrary*>(this);
            return SLANG_OK;
        }
        if (uuid == ISlangCastable::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangCastable*>(static_cast<ISlangSharedLibrary*>(this));
            return SLANG_OK;
        }
        if (uuid == ISlangUnknown::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangUnknown*>(static_cast<ISlangSharedLibrary*>(this));
            return SLANG_OK;
        }
        return m_actual->queryInterface(uuid, outObject);
    }

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) override
    {
        SLANG_UNUSED(guid);
        REPLAY_UNIMPLEMENTED_X("SharedLibraryProxy::castAs");
    }

    // ISlangSharedLibrary
    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name) override
    {
        RECORD_CALL();
        RECORD_INPUT(name);
        // Note: We don't record the return value since it's a raw pointer address
        // that would be meaningless during replay.
        return getActual<ISlangSharedLibrary>()->findSymbolAddressByName(name);
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_SHARED_LIBRARY_H
