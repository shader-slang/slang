#ifndef SLANG_PROXY_BLOB_H
#define SLANG_PROXY_BLOB_H

#include "proxy-base.h"
#include "proxy-macros.h"

namespace SlangRecord
{

class BlobProxy : public ProxyBase<ISlangBlob>
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

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(BlobProxy)

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE
    {
        if (!outObject) return SLANG_E_INVALID_ARG;

        if (uuid == BlobProxy::getTypeGuid() ||
            uuid == ISlangBlob::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangBlob*>(this);
            return SLANG_OK;
        }
        if (uuid == ISlangUnknown::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangUnknown*>(static_cast<ISlangBlob*>(this));
            return SLANG_OK;
        }
        return m_actual->queryInterface(uuid, outObject);
    }

    virtual SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() override
    {
        return getActual<ISlangBlob>()->getBufferPointer();
    }

    virtual SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() override
    {
        return getActual<ISlangBlob>()->getBufferSize();
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_BLOB_H
