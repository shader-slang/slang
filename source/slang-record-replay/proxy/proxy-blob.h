#ifndef SLANG_PROXY_BLOB_H
#define SLANG_PROXY_BLOB_H

#include "proxy-base.h"

#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class BlobProxy : public ISlangBlob, public ProxyBase
{
public:
    SLANG_COM_INTERFACE(
        0x4c5b8d3a,
        0x1e2f,
        0x4a9b,
        {0x8c, 0x7d, 0x6e, 0x3f, 0x2a, 0x1b, 0x0c, 0x9d})

    explicit BlobProxy(ISlangBlob* actual)
        : ProxyBase(actual)
    {
    }

    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ISlangUnknown* getInterface(const Guid& guid);

    virtual SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() override
    {
        SLANG_UNIMPLEMENTED_X("BlobProxy::getBufferPointer");
    }

    virtual SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() override
    {
        SLANG_UNIMPLEMENTED_X("BlobProxy::getBufferSize");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_BLOB_H
