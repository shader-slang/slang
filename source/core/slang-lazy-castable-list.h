// slang-lazy-castable-list.h
#ifndef SLANG_LAZY_CASTABLE_LIST_H
#define SLANG_LAZY_CASTABLE_LIST_H

#include "slang-castable-list.h"

#include "../../slang-com-ptr.h"

namespace Slang
{

/* Sometimes the overhead around having a potential list of items that might often be 
empty or only contain a single element is considerable. 

The `LazyCastableList` provides functionality around ICastableList to minimize allocation.
It does this by having a single m_castable member, which might be

* nullptr - means the list is empty
* pointing to something that *isn't* a ICastable list - then the list contains a single element that is that value
* pointing to a ICastableList, then the lists contents is the contents of the list

The methods will automatically convert the backing representation to something appropriate. 

Note! This means adding a ICastableList can cause problems, this is asserted for in add. 

If adding a ICastableList to the list is required, this can be achieved via calling `requireList` and adding the list.
*/
class LazyCastableList
{
public:
        /// Add a castable to the lsit
        /// Note! Do not use this to add a ICastableList, read the description
    void add(ICastable* castable);
        /// Return the amount of items in the list
    Count getCount() const;
        /// Remove the item at the specified index
    void removeAt(Index index);
        /// Clear the list
    void clear();
        /// Clear and deallocate the list
    void clearAndDeallocate();
        /// Find the first item that castAs(guid) produces a result
    void* find(const Guid& guid);
        /// Get the contents of the list as a view
    ConstArrayView<ICastable*> getView() const;
        /// Get the index of castable in the list. Returns -1 if not found
    Index indexOf(ICastable* castable) const;
        /// Get the index of unk. Handles if the wrapping has been used.
    Index indexOfUnknown(ISlangUnknown* unk) const;

        /// Will always return a valid ICastableList
    ICastableList* requireList();
        /// Will return nullptr if the list is empty, else it returns a ICastableList holding the elements
    ICastableList* getList();

protected:
    ComPtr<ICastable> m_castable;
};

} // namespace Slang

#endif
