// slang-lazy-castable-list.h
#ifndef SLANG_LAZY_CASTABLE_LIST_H
#define SLANG_LAZY_CASTABLE_LIST_H

#include "slang-castable-list.h"

#include "../../slang-com-ptr.h"

namespace Slang
{

/* Sometimes the overhead around having a potential list of items that might often be 
empty or only contain a single element is considerable. 

The `LazyCastableList` provides functionality around ICastableList to minimize allocation, or the 
need to allocate an ICastableList. It does this by tracking state in m_state, and varying the 
meaning of m_castable. 

* State::None - there is no list 
* State::One - there is a single entry, that is held in m_castable
* State::List - m_castable is actually ICastableList, and holds the contents
*/
class LazyCastableList
{
public:
        /// Add a castable to the lsit
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
        /// Find first match using predicate function
    ICastable* findWithPredicate(ICastableList::FindFunc func, void* data);
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
    enum class State
    {
        None,
        One,
        List,
    };
    // A state is not *strictly* necessary, because we can always determine what m_castable is
    // with a castAs. But doing so is not exactly fast, and using the state makes some code simpler
    // additionally.
    State m_state = State::None;
    ComPtr<ICastable> m_castable;
};

} // namespace Slang

#endif
