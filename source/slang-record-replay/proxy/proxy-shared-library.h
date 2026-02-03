#ifndef SLANG_PROXY_SHARED_LIBRARY_H
#define SLANG_PROXY_SHARED_LIBRARY_H

#include "proxy-base.h"

#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class SharedLibraryProxy : public ISlangSharedLibrary, public ProxyBase
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

    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ISlangUnknown* getInterface(const Guid& guid);

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) override
    {
        SLANG_UNUSED(guid);
        SLANG_UNIMPLEMENTED_X("SharedLibraryProxy::castAs");
    }

    // ISlangSharedLibrary
    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name) override
    {
        SLANG_UNUSED(name);
        SLANG_UNIMPLEMENTED_X("SharedLibraryProxy::findSymbolAddressByName");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_SHARED_LIBRARY_H
