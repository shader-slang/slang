// slang-castable-list-impl.h
#ifndef SLANG_CASTABLE_LIST_IMPL_H
#define SLANG_CASTABLE_LIST_IMPL_H

#include "slang-castable-list.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"

namespace Slang
{

/* Implementation of the ICastableList interface. 
Is atomic reference counted */
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
    virtual ICastable* SLANG_MCALL findWithPredicate(FindFunc func, void* data) SLANG_OVERRIDE;
    virtual ICastable*const* SLANG_MCALL getBuffer() SLANG_OVERRIDE { return m_list.getBuffer(); }

    static ComPtr<ICastableList> create() { return ComPtr<ICastableList>(new CastableList); }

        /// Dtor
    virtual ~CastableList();

    void* getInterface(const Guid& guid);
    void* getObject(const Guid& guid);

protected:

    List<ICastable*> m_list;
};

} // namespace Slang

#endif
