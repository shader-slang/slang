// slang-ast-reflect.h

#ifndef SLANG_AST_REFLECT_H
#define SLANG_AST_REFLECT_H

#include "slang-ast-generated.h"

// Switch based on node type - only inner and leaf classes define override
#define SLANG_AST_OVERRIDE_BASE
#define SLANG_AST_OVERRIDE_INNER override
#define SLANG_AST_OVERRIDE_LEAF override

// Switch based on the origin - classes defined in slang-ast-base.h do not have accept defined on them
// The 'origin' is based on the file where the class definition is located.

// Define what the accept method looks like, so don't have to repeat
#define SLANG_AST_ACCEPT_IMPL(NAME) virtual void accept(NAME::Visitor* visitor, void* extra) override;

// A macro for *all* of the different origin/files where AST files are defined. 
#define SLANG_AST_ACCEPT_BASE(NAME) 
#define SLANG_AST_ACCEPT_DECL(NAME) SLANG_AST_ACCEPT_IMPL(NAME)
#define SLANG_AST_ACCEPT_EXPR(NAME) SLANG_AST_ACCEPT_IMPL(NAME)
#define SLANG_AST_ACCEPT_MODIFIER(NAME) SLANG_AST_ACCEPT_IMPL(NAME)
#define SLANG_AST_ACCEPT_TYPE(NAME) SLANG_AST_ACCEPT_IMPL(NAME)
#define SLANG_AST_ACCEPT_VAL(NAME) SLANG_AST_ACCEPT_IMPL(NAME)
#define SLANG_AST_ACCEPT_STMT(NAME) SLANG_AST_ACCEPT_IMPL(NAME)

// Implementation for SLANG_ABSTRACT_CLASS(x) using reflection from C++ extractor in slang-ast-generated.h
#define SLANG_ABSTRACT_CLASS_REFLECT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    public:     \
    typedef SUPER Super; \
    static const ASTNodeType kType = ASTNodeType::NAME; \
    static const ReflectClassInfo kReflectClassInfo;  \
    SLANG_FORCE_INLINE static bool isDerivedFrom(ASTNodeType type) { return int(type) >= int(kType) && int(type) <= int(ASTNodeType::LAST); } \
    virtual const ReflectClassInfo& getClassInfo() const SLANG_AST_OVERRIDE_##TYPE { return kReflectClassInfo; } \

// Implementation for SLANG_CLASS(x) using reflection from C++ extractor in slang-ast-generated.h.
// This implementation is the same as for SLANG_ABSTRACT_CLASS_REFLECT_IMPL, except for the SLANG_AST_ACCEPT_ line which inserts the accept
// method for non 'BASE' origin classes. 
#define SLANG_CLASS_REFLECT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    public:     \
    typedef SUPER Super; \
    static const ASTNodeType kType = ASTNodeType::NAME; \
    static const ReflectClassInfo kReflectClassInfo;  \
    SLANG_FORCE_INLINE static bool isDerivedFrom(ASTNodeType type) { return int(type) >= int(kType) && int(type) <= int(ASTNodeType::LAST); } \
    virtual const ReflectClassInfo& getClassInfo() const SLANG_AST_OVERRIDE_##TYPE { return kReflectClassInfo; } \
    SLANG_AST_ACCEPT_##ORIGIN(NAME)

// Macro definitions - use the SLANG_ASTNode_ definitions to invoke the IMPL to produce the code
// injected into AST classes
#define SLANG_ABSTRACT_CLASS(NAME)  SLANG_ASTNode_##NAME(SLANG_ABSTRACT_CLASS_REFLECT_IMPL, _)
#define SLANG_CLASS(NAME)           SLANG_ASTNode_##NAME(SLANG_CLASS_REFLECT_IMPL, _)

// Does nothing - just a mark to the C++ extractor
#define SLANG_REFLECT_BASE_CLASS(NAME)
#define SLANG_REFLECTED
#define SLANG_UNREFLECTED

#endif // SLANG_AST_REFLECT_H
