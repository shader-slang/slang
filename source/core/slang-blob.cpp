#include "slang-blob.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BlobBase !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

ISlangUnknown* BlobBase::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() || 
        guid == ISlangBlob::getTypeGuid())
    {
        return static_cast<ISlangBlob*>(this);
    }
    if (guid == ICastable::getTypeGuid())
    {
        return static_cast<ICastable*>(this);
    }
    return nullptr;
}

void* BlobBase::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

void* BlobBase::castAs(const SlangUUID& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StaticBlob !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

SlangResult StaticBlob::queryInterface(SlangUUID const& guid, void** outObject) 
{
    if (auto intf = getInterface(guid))
    {
        *outObject = intf;
        return SLANG_OK;
    }
    return SLANG_E_NO_INTERFACE;
}

void* StaticBlob::castAs(const SlangUUID& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

ISlangUnknown* StaticBlob::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() ||
        guid == ISlangBlob::getTypeGuid())
    {
        return static_cast<ISlangBlob*>(this);
    }
    if (guid == ICastable::getTypeGuid())
    {
        return static_cast<ICastable*>(this);
    }
    return nullptr;
}

void* StaticBlob::getObject(const Guid& guid)
{
    SLANG_UNUSED(guid);
    return nullptr;
}

} // namespace Slang
