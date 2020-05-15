// slang-ast-reflect.h

#ifndef SLANG_AST_REFLECT_H
#define SLANG_AST_REFLECT_H

#include "slang-ast-generated.h"

#define SLANG_AST_OVERRIDE_BASE
#define SLANG_AST_OVERRIDE_INNER override
#define SLANG_AST_OVERRIDE_LEAF override

// Implement the part that uses the class definition
#define SLANG_CLASS_REFLECT_DEFAULT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    typedef SUPER Super; \
    SLANG_FORCE_INLINE static bool isDerivedFrom(ASTNodeType type) { return int(type) >= int(kType) && int(type) <= int(ASTNodeType::LAST); } \
    virtual const ReflectClassInfo& getClassInfo() const SLANG_AST_OVERRIDE_##TYPE { return kReflectClassInfo; } \


#define SLANG_CLASS_REFLECT_DEFAULT(NAME) \
    public:     \
    static const ASTNodeType kType = ASTNodeType::NAME; \
    static const ReflectClassInfo kReflectClassInfo; \
    SLANG_ASTNode_##NAME(SLANG_CLASS_REFLECT_DEFAULT_IMPL, _)

#define SLANG_CLASS_REFLECT_WITH_ACCEPT(NAME) \
    SLANG_CLASS_REFLECT_DEFAULT(NAME) \
    virtual void accept(NAME::Visitor* visitor, void* extra) override;

#define SLANG_ABSTRACT_CLASS_REFLECT(NAME)  SLANG_CLASS_REFLECT_DEFAULT(NAME)
#define SLANG_CLASS_REFLECT(NAME) SLANG_CLASS_REFLECT_DEFAULT(NAME)

// Used for C++ extractor, does nothing here
#define SLANG_REFLECT_BASE_CLASS(x) /* ... */
#define SLANG_REFLECTED
#define SLANG_UNREFLECTED

#endif // SLANG_AST_REFLECT_H
