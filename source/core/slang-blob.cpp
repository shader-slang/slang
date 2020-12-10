#include "slang-blob.h"

namespace Slang {

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangBlob = SLANG_UUID_ISlangBlob;

ISlangUnknown* BlobBase::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangBlob) ? static_cast<ISlangBlob*>(this) : nullptr;
}

SlangResult StaticBlob::queryInterface(SlangUUID const& guid, void** outObject) 
{
    if (guid == IID_ISlangUnknown || guid == IID_ISlangBlob)
    {
        *outObject = static_cast<ISlangBlob*>(this);
        return SLANG_OK;
    }
    return SLANG_E_NO_INTERFACE;
}
 
} // namespace Slang
