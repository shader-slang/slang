#ifndef SLANG_PROXY_METADATA_H
#define SLANG_PROXY_METADATA_H

#include "proxy-base.h"

#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class MetadataProxy : public slang::IMetadata, public ProxyBase
{
public:
    SLANG_COM_INTERFACE(
        0x5d6c9e4b,
        0x2f3a,
        0x5b0c,
        {0x9d, 0x8e, 0x7f, 0x4a, 0x3b, 0x2c, 0x1d, 0xae})

    explicit MetadataProxy(slang::IMetadata* actual)
        : ProxyBase(actual)
    {
    }

    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ISlangUnknown* getInterface(const Guid& guid);

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) override
    {
        SLANG_UNUSED(guid);
        SLANG_UNIMPLEMENTED_X("MetadataProxy::castAs");
    }

    // IMetadata
    virtual SlangResult isParameterLocationUsed(
        SlangParameterCategory category,
        SlangUInt spaceIndex,
        SlangUInt registerIndex,
        bool& outUsed) override
    {
        SLANG_UNUSED(category);
        SLANG_UNUSED(spaceIndex);
        SLANG_UNUSED(registerIndex);
        SLANG_UNUSED(outUsed);
        SLANG_UNIMPLEMENTED_X("MetadataProxy::isParameterLocationUsed");
    }

    virtual const char* SLANG_MCALL getDebugBuildIdentifier() override
    {
        SLANG_UNIMPLEMENTED_X("MetadataProxy::getDebugBuildIdentifier");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_METADATA_H
