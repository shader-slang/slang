// slang-lazy-castable-list.h
#ifndef SLANG_LAZY_CASTABLE_LIST_H
#define SLANG_LAZY_CASTABLE_LIST_H

#include "slang-castable-list.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-com-object.h"

namespace Slang
{

class LazyCastableList
{
public:
    void add(ICastable* castable);
    Count getCount() const;
    void removeAt(Index index);
    void clear();
    void clearAndDeallocate();
    void* find(const Guid& guid);
    ConstArrayView<ICastable*> getView() const;
    Index indexOf(ICastable* castable) const;
    Index indexOfUnknown(ISlangUnknown* unk) const;

    ICastableList* requireList();
    ICastableList* getList();

protected:
    ComPtr<ICastable> m_castable;
};

} // namespace Slang

#endif
