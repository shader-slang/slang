#ifndef SLANG_COM_OBJECT_H
#define SLANG_COM_OBJECT_H

#include "slang-basic.h"
#include <atomic>

namespace Slang
{
class ComObject : public RefObject
{
protected:
    std::atomic<uint32_t> comRefCount;

public:
    ComObject()
        : comRefCount(0)
    {}
    ComObject(const ComObject& rhs) :
        RefObject(rhs),
        comRefCount(0) 
    {}

    ComObject& operator=(const ComObject&) { return *this; }

    virtual void comFree() {}

    uint32_t addRefImpl()
    {
        auto oldRefCount = comRefCount++;
        if (oldRefCount == 0)
            addReference();
        return oldRefCount + 1;
    }

    uint32_t releaseImpl()
    {
        auto oldRefCount = comRefCount--;
        if (oldRefCount == 1)
        {
            comFree();
            releaseReference();
        }
        return oldRefCount - 1;
    }
};

#define SLANG_COM_OBJECT_IUNKNOWN_QUERY_INTERFACE                                                  \
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) \
        SLANG_OVERRIDE                                                                             \
    {                                                                                              \
        void* intf = getInterface(uuid);                                                           \
        if (intf)                                                                                  \
        {                                                                                          \
            addRef();                                                                              \
            *outObject = intf;                                                                     \
            return SLANG_OK;                                                                       \
        }                                                                                          \
        return SLANG_E_NO_INTERFACE;                                                               \
    }
#define SLANG_COM_OBJECT_IUNKNOWN_ADD_REF \
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return addRefImpl(); }
#define SLANG_COM_OBJECT_IUNKNOWN_RELEASE \
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return releaseImpl(); }
#define SLANG_COM_OBJECT_IUNKNOWN_ALL         \
    SLANG_COM_OBJECT_IUNKNOWN_QUERY_INTERFACE \
    SLANG_COM_OBJECT_IUNKNOWN_ADD_REF         \
    SLANG_COM_OBJECT_IUNKNOWN_RELEASE

} // namespace Slang

#endif
