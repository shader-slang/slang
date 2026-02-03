#ifndef SLANG_PROXY_COMPILE_RESULT_H
#define SLANG_PROXY_COMPILE_RESULT_H

#include "proxy-base.h"

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

    // ISlangCastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) override
    {
        SLANG_UNUSED(guid);
        SLANG_UNIMPLEMENTED_X("CompileResultProxy::castAs");
    }

    // ICompileResult
    virtual uint32_t SLANG_MCALL getItemCount() override
    {
        SLANG_UNIMPLEMENTED_X("CompileResultProxy::getItemCount");
    }

    virtual SlangResult SLANG_MCALL getItemData(uint32_t index, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(index);
        SLANG_UNUSED(outBlob);
        SLANG_UNIMPLEMENTED_X("CompileResultProxy::getItemData");
    }

    virtual SlangResult SLANG_MCALL getMetadata(slang::IMetadata** outMetadata) override
    {
        SLANG_UNUSED(outMetadata);
        SLANG_UNIMPLEMENTED_X("CompileResultProxy::getMetadata");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_COMPILE_RESULT_H
