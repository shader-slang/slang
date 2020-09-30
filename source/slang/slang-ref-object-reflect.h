// slang-ref-object-reflect.h

#ifndef SLANG_REF_OBJECT_REFLECT_H
#define SLANG_REF_OBJECT_REFLECT_H

#include "slang-serialize-reflection.h"

#include "slang-ref-object-generated.h"

#include "../core/slang-smart-pointer.h"

struct RefObjectAccess; 

#define SLANG_OBJ_CLASS_REFLECT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    public:     \
    typedef NAME This; \
    static const ReflectClassInfo kReflectClassInfo;  \
    virtual const ReflectClassInfo* getClassInfo() SLANG_OVERRIDE { return &kReflectClassInfo; } \
    \
    friend struct RefObjectAccess; \
    \
    SLANG_CLASS_REFLECT_SUPER_##TYPE(SUPER) \
   
// Placed in any SerialRefObject derived class
#define SLANG_OBJ_ABSTRACT_CLASS(NAME)  SLANG_RefObject_##NAME(SLANG_OBJ_CLASS_REFLECT_IMPL, _)
#define SLANG_OBJ_CLASS(NAME)           SLANG_RefObject_##NAME(SLANG_OBJ_CLASS_REFLECT_IMPL, _)

//#define SLANG_OBJ_ABSTRACT_CLASS(NAME)
//#define SLANG_OBJ_CLASS(NAME)         

namespace Slang
{

// Is friended such that internally we have access to construct or get members
struct RefObjectAccess;

class SerialRefObject : public RefObject
{
public:
    typedef RefObject Super;
    typedef SerialRefObject This;

    static ReflectClassInfo kReflectClassInfo;

    virtual const ReflectClassInfo* getClassInfo() { return &kReflectClassInfo; }
};

} // namespace Slang

#endif // SLANG_REF_OBJECT_REFLECT_H
