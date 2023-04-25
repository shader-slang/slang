// slang-serialize-reflection.cpp
#include "slang-serialize-reflection.h"

#include "slang-serialize.h"

namespace Slang {

bool ReflectClassInfo::isSubClassOfSlow(const ThisType& super) const
{
    ReflectClassInfo const* info = this;
    while (info)
    {
        if (info == &super)
            return true;
        info = info->m_superClass;
    }
    return false;
}

#if 0

// #if'd out because produces a warning->error if not used.
static bool _checkSubClassRange(ReflectClassInfo*const* typeInfos, Index typeInfosCount)
{   
    for (Index i = 0; i < typeInfosCount; ++i)
    {
        for (Index j = 0; j < typeInfosCount; ++j)
        {
            auto a = typeInfos[i];
            auto b = typeInfos[j];
            if (a->isSubClassOf(*b) != a->isSubClassOfSlow(*b))
            {
                return false;
            }
        }
    }

    return true;
}

#endif

static uint32_t _calcRangeRec(ReflectClassInfo* classInfo, const Dictionary<const ReflectClassInfo*, List<ReflectClassInfo*> >& childMap, uint32_t index)
{
    classInfo->m_classId = index++;
    // Do the calc range for all the children
    auto list = childMap.tryGetValue(classInfo);

    if (list)
    {
        for (auto child : *list)
        {
            index = _calcRangeRec(child, childMap, index);
        }
    }

    classInfo->m_lastClassId = index;
    return index;
}

static ReflectClassInfo* _calcRoot(ReflectClassInfo* classInfo)
{
    while (classInfo->m_superClass)
    {
        classInfo = const_cast<ReflectClassInfo*>(classInfo->m_superClass);
    }
    return classInfo;
}


/* static */void ReflectClassInfo::calcClassIdHierachy(uint32_t baseIndex, ReflectClassInfo*const* typeInfos, Index typeInfosCount)
{
    SLANG_ASSERT(typeInfosCount > 0);

    // TODO(JS):
    // Note that the calculating of the ranges could be done more efficiently by adding to an array of struct { super, class }, sorting, by super classs
    // and using a dictionary to map from class it's first in list of super class use. This works for now though.

    // The root cannot be shared with another hierarchy - as doing so will mean that the range will be incorrect (it would need to span both trees)
    ReflectClassInfo* root = _calcRoot(typeInfos[0]);

    // We want to produce a map from a node that holds all of it's children
    Dictionary<const ThisType*, List<ThisType*> > childMap;

    const List<ThisType*> emptyList;
    {
        for (Index i = 0; i < typeInfosCount; ++ i)
        {
            auto typeInfo = typeInfos[i];
            if (typeInfo->m_superClass)
            {
                // Add to that item
                List<ThisType*>* list = childMap.tryGetValueOrAdd(typeInfo->m_superClass, emptyList);
                if (!list)
                {
                    list = childMap.tryGetValue(typeInfo->m_superClass);
                }
                SLANG_ASSERT(list);
                list->add(typeInfo);
            }

            // The root should be the same for all types
            SLANG_ASSERT(_calcRoot(typeInfo) == root);
        }
    }

    // We want to recursively work out a range
    _calcRangeRec(root, childMap, baseIndex);

    //SLANG_ASSERT(_checkSubClassRange(typeInfos, typeInfoCount));
}

} // namespace Slang
