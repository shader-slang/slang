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

void StringBlob::_uniqueInit(StringRepresentation* uniqueRep)
{
    // When initing the rep either has to be nullptr *or* uniquely referenced
    // so we can take ownership of it.

    SLANG_ASSERT(uniqueRep == nullptr || uniqueRep->isUniquelyReferenced());

    m_rep = uniqueRep;

    // If it's nullptr, that means the string is the empty string. To handle we 
    // 
    if (!uniqueRep)
    {
        m_chars = "";
        m_charsCount = 0;
    }
    else
    {
        m_chars = uniqueRep->getData();
        m_charsCount = uniqueRep->getLength();
    }

    _checkRep();
}

void StringBlob::_init(StringRepresentation* rep)
{
    if (rep)
    {
        // Even if the uniqueRep is 1, we can't assume we can take ownership
        // So we make a copy

        // If the length is 0, we can just init as empty
        auto length = rep->getLength();
        if (length == 0)
        {
            rep = nullptr;
        }
        else
        {
            const UnownedStringSlice slice(rep->getData(), length);
            rep = StringRepresentation::createWithReference(slice);
        }
    }

    // Must be unique at this point
    _uniqueInit(rep);

    _checkRep();
}

void StringBlob::_moveInit(StringRepresentation* rep)
{
    if (rep && !rep->isUniquelyReferenced())
    {
        // Will make a copy of the rep
        _init(rep);
        // We need to release a ref as rep is passed in with the 'current' ref count 
        rep->releaseReference();
    }
    else
    {
        _uniqueInit(rep);
    }
    _checkRep();
}

/* static */ComPtr<ISlangBlob> StringBlob::create(const UnownedStringSlice& slice)
{
    StringRepresentation* rep = nullptr;
    if (slice.getLength())
    {
        rep = StringRepresentation::createWithReference(slice);
    }

    // rep must be unique at this point
    auto blob = new StringBlob;
    blob->_uniqueInit(rep);
    return ComPtr<ISlangBlob>(blob);
}

/* static */ComPtr<ISlangBlob> StringBlob::create(const String& in)
{
    auto blob = new StringBlob;
    blob->_init(in.getStringRepresentation());
    return ComPtr<ISlangBlob>(blob);
}

/* static */ComPtr<ISlangBlob> StringBlob::moveCreate(String& in)
{
    auto blob = new StringBlob;
    blob->_moveInit(in.detachStringRepresentation());
    return ComPtr<ISlangBlob>(blob);
}

/* static */ComPtr<ISlangBlob> StringBlob::moveCreate(String&& in)
{
    auto blob = new StringBlob;
    blob->_moveInit(in.detachStringRepresentation());
    return ComPtr<ISlangBlob>(blob);
}

StringBlob::~StringBlob()
{
    if (m_rep)
    {
        delete m_rep;
    }
}
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
        return const_cast<char*>(m_chars);
    }
    return nullptr;
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
