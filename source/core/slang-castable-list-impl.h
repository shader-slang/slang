// slang-castable-list.h
#ifndef SLANG_CASTABLE_LIST_IMPL_H
#define SLANG_CASTABLE_LIST_IMPL_H

#include "slang-castable-list.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"

namespace Slang
{

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

/* Implementation of the ICastableList interface. 
Is atomic reference counted*/
class CastableList : public ComBaseObject, public ICastableList
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) SLANG_OVERRIDE;

    // ICastableList
    virtual Count SLANG_MCALL getCount() SLANG_OVERRIDE { return m_list.getCount(); }
    virtual ICastable* SLANG_MCALL getAt(Index i) SLANG_OVERRIDE { return m_list[i]; }
    virtual void SLANG_MCALL add(ICastable* castable) SLANG_OVERRIDE;
    virtual void SLANG_MCALL addUnknown(ISlangUnknown* unk) SLANG_OVERRIDE;
    virtual void SLANG_MCALL removeAt(Index i) SLANG_OVERRIDE;
    virtual void SLANG_MCALL clear() SLANG_OVERRIDE;
    virtual Index SLANG_MCALL indexOf(ICastable* castable) SLANG_OVERRIDE;
    virtual Index SLANG_MCALL indexOfUnknown(ISlangUnknown* unk) SLANG_OVERRIDE;
    virtual void* SLANG_MCALL find(const Guid& guid) SLANG_OVERRIDE;
    virtual ICastable*const* SLANG_MCALL getBuffer() SLANG_OVERRIDE { return m_list.getBuffer(); }

        /// Dtor
    virtual ~CastableList();

protected:
    void* getInterface(const Guid& guid);
    void* getObject(const Guid& guid);

    List<ICastable*> m_list;
};

} // namespace Slang

#endif
