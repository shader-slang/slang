#include "slang-object-scope-manager.h"

namespace Slang {

void ObjectScopeManager::_releaseAll()
{
    RefObject*const* objs = m_objs.begin();
    const int numObjs = int(m_objs.getSize());
    for (int i = 0; i < numObjs; ++i)
    {
        objs[i]->decreaseReference();
    }
}

void ObjectScopeManager::clear()
{
    _releaseAll();
    // Free the memory as well as resizing
    m_objs = List<RefObject*>();
}

} // namespace Slang

