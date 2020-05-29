// slang-ast-reflect.h

#ifndef SLANG_AST_REFLECT_H
#define SLANG_AST_REFLECT_H

#include "slang-ast-generated.h"

// Implementation for SLANG_ABSTRACT_CLASS(x) using reflection from C++ extractor in slang-ast-generated.h
#define SLANG_CLASS_REFLECT_IMPL(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    protected: \
    NAME() = default; \
    public:     \
    typedef NAME This; \
    typedef SUPER Super; \
    static const ASTNodeType kType = ASTNodeType::NAME; \
    static const ReflectClassInfo kReflectClassInfo;  \
    SLANG_FORCE_INLINE static bool isDerivedFrom(ASTNodeType type) { return int(type) >= int(kType) && int(type) <= int(ASTNodeType::LAST); } \
    friend class ASTBuilder; \
    friend struct ASTConstructAccess; 

// Macro definitions - use the SLANG_ASTNode_ definitions to invoke the IMPL to produce the code
// injected into AST classes
#define SLANG_ABSTRACT_CLASS(NAME)  SLANG_ASTNode_##NAME(SLANG_CLASS_REFLECT_IMPL, _)
#define SLANG_CLASS(NAME)           SLANG_ASTNode_##NAME(SLANG_CLASS_REFLECT_IMPL, _)

// Does nothing - just a mark to the C++ extractor
#define SLANG_REFLECT_BASE_CLASS(NAME)
#define SLANG_REFLECTED
#define SLANG_UNREFLECTED

#endif // SLANG_AST_REFLECT_H
