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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StringBlob !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* StringBlob::castAs(const SlangUUID& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

void* StringBlob::getObject(const Guid& guid)
{
    // Can allow accessing the contained String
    if (guid == getTypeGuid())
    {
        return this;
    }
    // Can always be accessed as terminated char*
    if (guid == SlangTerminatedChars::getTypeGuid())
    {
        return const_cast<char*>(m_string.getBuffer());
    }
    return nullptr;
}

void StringBlob::_moveUnique(String& in)
{
    auto rep = in.getStringRepresentation();
    if (rep && !rep->isUniquelyReferenced())
    {
        // Make a new unique copy
        m_string = in.getUnownedSlice();

        // Move out of in
        String tmp;
        tmp.swapWith(in);
    }
    else
    {
        // Must either not have a rep or be unique
        m_string.swapWith(in);
    }
}

/* static */ComPtr<ISlangBlob> StringBlob::moveCreate(String& in)
{
    auto blob = new StringBlob;
    blob->_moveUnique(in);
    return ComPtr<ISlangBlob>(blob);
}

/* static */ComPtr<ISlangBlob> StringBlob::moveCreate(String&& in)
{
    auto blob = new StringBlob;
    blob->_moveUnique(in);
    return ComPtr<ISlangBlob>(blob);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! RawBlob !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* RawBlob::castAs(const SlangUUID& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

void* RawBlob::getObject(const Guid& guid)
{
    // If the data has 0 termination, we can return the pointer
    if (guid == SlangTerminatedChars::getTypeGuid() && m_data.isTerminated())
    {
        return (char*)m_data.getData(); 
    }
    return nullptr;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ScopeBlob !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* ScopeBlob::castAs(const SlangUUID& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    if (auto obj = getObject(guid))
    {
        return obj;
    }

    // If the contained thing is castable, ask it 
    if (m_castable)
    {
        return m_castable->castAs(guid);
    }

    return nullptr;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ListBlob !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void* ListBlob::castAs(const SlangUUID& guid)
{
    if (auto intf = getInterface(guid))
    {
        return intf;
    }
    return getObject(guid);
}

void* ListBlob::getObject(const Guid& guid)
{
    // If the data is terminated return the pointer
    if (guid == SlangTerminatedChars::getTypeGuid())
    {
        const auto count = m_data.getCount();
        if (m_data.getCapacity() > count)
        {
            auto buf = m_data.getBuffer();
            if (buf[count] == 0)
            {
                return (char*)buf;
            }
        }
    }
    return nullptr;
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
