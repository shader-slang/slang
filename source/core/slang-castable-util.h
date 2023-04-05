// slang-castable-util.h
#ifndef SLANG_CASTABLE_UTIL_H
#define SLANG_CASTABLE_UTIL_H


#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"

namespace Slang
{

/* Adapter interface to make a non castable types work as ICastable */
class IUnknownCastableAdapter : public ICastable
{
    SLANG_COM_INTERFACE(0x8b4aad81, 0x4934, 0x4a67, { 0xb2, 0xe2, 0xe9, 0x17, 0xfc, 0x29, 0x12, 0x54 });

    /// When using the adapter, this provides a way to directly get the internal no ICastable type
    virtual SLANG_NO_THROW ISlangUnknown* SLANG_MCALL getContained() = 0;
};

/* An adapter such that types which aren't derived from ICastable, can be used as such. 

With the following caveats.
* the interfaces/objects of the adapter are checked *first*, so IUnknown will always be for the adapter
* assumes when doing a queryInterface on the contained item, it will remain in scope when released (this is *not* strict COM)
*/
class UnknownCastableAdapter : public ComBaseObject, public IUnknownCastableAdapter
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    // IUnknownCastableAdapter
    virtual SLANG_NO_THROW ISlangUnknown* SLANG_MCALL getContained() SLANG_OVERRIDE { return m_contained; }

    UnknownCastableAdapter(ISlangUnknown* unk):
        m_contained(unk)
    {
        SLANG_ASSERT(unk);
    }

protected:
    void* getInterface(const Guid& guid);
    void* getObject(const Guid& guid);

    ComPtr<ISlangUnknown> m_contained;

    // We hold a cache for a single lookup to make things a little faster
    void* m_found = nullptr;
    Guid m_foundGuid;
};

struct CastableUtil
{
        /// Given an ISlangUnkown return as a castable interface. 
        /// Can use UnknownCastableAdapter if can't queryInterface unk to ICastable
    static ComPtr<ICastable> getCastable(ISlangUnknown* unk);
};

} // namespace Slang

#endif
