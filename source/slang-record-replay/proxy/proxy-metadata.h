#ifndef SLANG_PROXY_METADATA_H
#define SLANG_PROXY_METADATA_H

#include "proxy-base.h"
#include "proxy-macros.h"

#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class MetadataProxy : public ProxyBase<slang::IMetadata>
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

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(MetadataProxy)

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE
    {
        if (!outObject) return SLANG_E_INVALID_ARG;

        if (uuid == MetadataProxy::getTypeGuid() ||
            uuid == slang::IMetadata::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::IMetadata*>(this);
            return SLANG_OK;
        }
        if (uuid == ISlangCastable::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangCastable*>(static_cast<slang::IMetadata*>(this));
            return SLANG_OK;
        }
        if (uuid == ISlangUnknown::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangUnknown*>(static_cast<slang::IMetadata*>(this));
            return SLANG_OK;
        }
        return m_actual->queryInterface(uuid, outObject);
    }

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) override
    {
        SLANG_UNUSED(guid);
        REPLAY_UNIMPLEMENTED_X("MetadataProxy::castAs");
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
        REPLAY_UNIMPLEMENTED_X("MetadataProxy::isParameterLocationUsed");
    }

    virtual const char* SLANG_MCALL getDebugBuildIdentifier() override
    {
        REPLAY_UNIMPLEMENTED_X("MetadataProxy::getDebugBuildIdentifier");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_METADATA_H
