// slang-ast-reflect.h

#ifndef SLANG_AST_REFLECT_H
#define SLANG_AST_REFLECT_H

#include "slang-ast-generated.h"

// Implement the part that uses the class definition
#define SLANG_CLASS_REFLECT_DEFAULT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    typedef SUPER Super; \
    SLANG_FORCE_INLINE static bool isDerivedFrom(ASTNodeType type) { return int(type) >= int(kType) && int(type) <= int(ASTNodeType::LAST); } \

#define SLANG_CLASS_REFLECT_DEFAULT(NAME) \
    public:     \
    static const ASTNodeType kType = ASTNodeType::NAME; \
    static const ReflectClassInfo kReflectClassInfo; \
    virtual const ReflectClassInfo& getClassInfo() const { return kReflectClassInfo; } \
    SLANG_ASTNode_##NAME(SLANG_CLASS_REFLECT_DEFAULT_IMPL, _)

#define SLANG_CLASS_REFLECT_WITH_ACCEPT(NAME) \
    SLANG_CLASS_REFLECT_DEFAULT(NAME) \
    virtual void accept(NAME::Visitor* visitor, void* extra) override;

#define SLANG_ABSTRACT_CLASS_REFLECT(NAME)  SLANG_CLASS_REFLECT_DEFAULT(NAME)
#define SLANG_CLASS_REFLECT(NAME) SLANG_CLASS_REFLECT_DEFAULT(NAME)

// Used for C++ extractor, does nothing here
#define SLANG_REFLECT_BASE_CLASS(x) /* ... */



#endif // SLANG_AST_REFLECT_H
