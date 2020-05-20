// slang-ast-reflect.h

#ifndef SLANG_AST_REFLECT_H
#define SLANG_AST_REFLECT_H

#include "slang-ast-generated.h"

// Switch based on node type - only inner and leaf classes define override
#define SLANG_AST_OVERRIDE_BASE
#define SLANG_AST_OVERRIDE_INNER override
#define SLANG_AST_OVERRIDE_LEAF override

// Implementation for SLANG_ABSTRACT_CLASS(x) using reflection from C++ extractor in slang-ast-generated.h
#define SLANG_CLASS_REFLECT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    public:     \
    typedef SUPER Super; \
    static const ASTNodeType kType = ASTNodeType::NAME; \
    static const ReflectClassInfo kReflectClassInfo;  \
    SLANG_FORCE_INLINE static bool isDerivedFrom(ASTNodeType type) { return int(type) >= int(kType) && int(type) <= int(ASTNodeType::LAST); } \
    virtual const ReflectClassInfo& getClassInfo() const SLANG_AST_OVERRIDE_##TYPE { return kReflectClassInfo; } \

// Macro definitions - use the SLANG_ASTNode_ definitions to invoke the IMPL to produce the code
// injected into AST classes
#define SLANG_ABSTRACT_CLASS(NAME)  SLANG_ASTNode_##NAME(SLANG_CLASS_REFLECT_IMPL, _)
#define SLANG_CLASS(NAME)           SLANG_ASTNode_##NAME(SLANG_CLASS_REFLECT_IMPL, _)

// Does nothing - just a mark to the C++ extractor
#define SLANG_REFLECT_BASE_CLASS(NAME)
#define SLANG_REFLECTED
#define SLANG_UNREFLECTED

#endif // SLANG_AST_REFLECT_H
