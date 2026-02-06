#ifndef SLANG_PROXY_COMPILE_RESULT_H
#define SLANG_PROXY_COMPILE_RESULT_H

#include "proxy-base.h"
#include "proxy-macros.h"

#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class CompileResultProxy : public ProxyBase<slang::ICompileResult>
{
public:
    SLANG_COM_INTERFACE(
        0x6e7d0f5c,
        0x3a4b,
        0x6c1d,
        {0xae, 0x9f, 0x8a, 0x5b, 0x4c, 0x3d, 0x2e, 0xbf})

    explicit CompileResultProxy(slang::ICompileResult* actual)
        : ProxyBase(actual)
    {
    }

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(CompileResultProxy)

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE
    {
        if (!outObject) return SLANG_E_INVALID_ARG;

        if (uuid == CompileResultProxy::getTypeGuid() ||
            uuid == slang::ICompileResult::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::ICompileResult*>(this);
            return SLANG_OK;
        }
        if (uuid == ISlangCastable::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangCastable*>(static_cast<slang::ICompileResult*>(this));
            return SLANG_OK;
        }
        if (uuid == ISlangUnknown::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangUnknown*>(static_cast<slang::ICompileResult*>(this));
            return SLANG_OK;
        }
        return m_actual->queryInterface(uuid, outObject);
    }

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) override
    {
        SLANG_UNUSED(guid);
        REPLAY_UNIMPLEMENTED_X("CompileResultProxy::castAs");
    }

    // ICompileResult
    virtual uint32_t SLANG_MCALL getItemCount() override
    {
        REPLAY_UNIMPLEMENTED_X("CompileResultProxy::getItemCount");
    }

    virtual SlangResult SLANG_MCALL getItemData(uint32_t index, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(index);
        SLANG_UNUSED(outBlob);
        REPLAY_UNIMPLEMENTED_X("CompileResultProxy::getItemData");
    }

    virtual SlangResult SLANG_MCALL getMetadata(slang::IMetadata** outMetadata) override
    {
        SLANG_UNUSED(outMetadata);
        REPLAY_UNIMPLEMENTED_X("CompileResultProxy::getMetadata");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_COMPILE_RESULT_H
