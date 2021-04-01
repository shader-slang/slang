#include "slang-blob.h"

namespace Slang {

ISlangUnknown* BlobBase::getInterface(const Guid& guid)
{
    return (guid == ISlangUnknown::getTypeGuid() || guid == ISlangBlob::getTypeGuid()) ? static_cast<ISlangBlob*>(this) : nullptr;
}

SlangResult StaticBlob::queryInterface(SlangUUID const& guid, void** outObject) 
{
    if (guid == ISlangUnknown::getTypeGuid() || guid == ISlangBlob::getTypeGuid())
    {
        *outObject = static_cast<ISlangBlob*>(this);
        return SLANG_OK;
    }
    return SLANG_E_NO_INTERFACE;
}
 
} // namespace Slang
