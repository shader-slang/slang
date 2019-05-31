#ifndef SLANG_CORE_OBJECT_SCOPE_MANAGER_H
#define SLANG_CORE_OBJECT_SCOPE_MANAGER_H

#include "slang-smart-pointer.h"
#include "slang-list.h"

namespace Slang {

/** Keep objects added in scope.

This is currently about the most simple implementation possible. Objects are added to a list
which members are released when ObjectScopeManager loses scope, or clear is called. 

The same object can be added multiple times. This implementation will just add the same object
multiple times. A more complex implementation might notice that the object is already in scope
and not add a reference. 

Another potential improvement would be to hold the pointers in a MemoryArena. Doing so would remove
the requirement of a List of contiguous memory. 

In implementations that can hold multiple references to the same thing, we may want to add some 
garbage collection to remove repeat references.
*/
struct ObjectScopeManager
{
public:

        /// Add an object which will be kept in scope until manager is destroyed
    SLANG_INLINE RefObject* add(RefObject* obj);
        /// Add an object, where it may be nullptr. If it null its a no-op
    SLANG_INLINE RefObject* addMaybeNull(RefObject* obj);

        /// Clear the contents
    void clear();

        /// Dtor
    ~ObjectScopeManager() { _releaseAll(); }

protected:
    void _releaseAll();

    List<RefObject*> m_objs;
};

// ---------------------------------------------------------------------------
RefObject* ObjectScopeManager::addMaybeNull(RefObject* obj)
{
    if (obj)
    {
        obj->addReference();
        m_objs.add(obj);
    }
    return obj;
}

// ---------------------------------------------------------------------------
RefObject* ObjectScopeManager::add(RefObject* obj)
{
    SLANG_ASSERT(obj);
    obj->addReference();
    m_objs.add(obj);
    return obj;
}

} // namespace Slang

#endif // SLANG_OBJECT_SCOPE_MANAGER_H
