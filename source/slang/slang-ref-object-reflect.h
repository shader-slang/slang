// slang-ref-object-reflect.h

#ifndef SLANG_REF_OBJECT_REFLECT_H
#define SLANG_REF_OBJECT_REFLECT_H

#include "slang-serialize-reflection.h"

#include "slang-generated-obj.h"

#include "../core/slang-smart-pointer.h"

class SerialClasses;

struct RefObjectAccess; 

#define SLANG_OBJ_CLASS_REFLECT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    public:     \
    typedef NAME This; \
    static const ReflectClassInfo kReflectClassInfo;  \
    virtual const ReflectClassInfo* getClassInfo() const SLANG_OVERRIDE { return &kReflectClassInfo; } \
    \
    friend struct RefObjectAccess; \
    \
    SLANG_CLASS_REFLECT_SUPER_##TYPE(SUPER) \
   
// Placed in any SerialRefObject derived class
#define SLANG_ABSTRACT_OBJ_CLASS(NAME)  SLANG_RefObject_##NAME(SLANG_OBJ_CLASS_REFLECT_IMPL, _)
#define SLANG_OBJ_CLASS(NAME)           SLANG_RefObject_##NAME(SLANG_OBJ_CLASS_REFLECT_IMPL, _)

namespace Slang
{

class SerialClasses;

// Is friended such that internally we have access to construct or get members
struct RefObjectAccess;

// Base class for Serialized RefObject derived classes. The main feature is that gives away to get ReflectClassInfo
// via getClassInfo() method
class SerialRefObject : public RefObject
{
public:
    typedef RefObject Super;
    typedef SerialRefObject This;

    static const ReflectClassInfo kReflectClassInfo;

    virtual const ReflectClassInfo* getClassInfo() const { return &kReflectClassInfo; }
};

// For turning RefObjectType back to ReflectClassInfo
struct SerialRefObjects
{
        /// Add serialization classes
    static SlangResult addSerialClasses(SerialClasses* serialClasses);

    static const ReflectClassInfo* getClassInfo(RefObjectType type) { return g_singleton.m_infos[Index(type)]; }

    
    static const SerialRefObjects g_singleton;

protected:
    SerialRefObjects();
    const ReflectClassInfo* m_infos[Index(RefObjectType::CountOf)];
};

} // namespace Slang

#endif // SLANG_REF_OBJECT_REFLECT_H
