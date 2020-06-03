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

// Macros for simulating virtual methods without virtual methods

#define SLANG_AST_NODE_INVOKE(method, methodParams) _##method##Override methodParams

#define SLANG_AST_NODE_CASE(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param)      case ASTNodeType::NAME: return static_cast<NAME*>(this)-> SLANG_AST_NODE_INVOKE param;

#define SLANG_AST_NODE_VIRTUAL_CALL(base, methodName, methodParams) \
    switch (astNodeType) \
    { \
        SLANG_ALL_ASTNode_##base(SLANG_AST_NODE_CASE, (methodName, methodParams)) \
        default: return SLANG_AST_NODE_INVOKE (methodName, methodParams); \
    }

// Same but for a method that's const
#define SLANG_AST_NODE_CONST_CASE(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param)      case ASTNodeType::NAME: return static_cast<const NAME*>(this)-> SLANG_AST_NODE_INVOKE param;
#define SLANG_AST_NODE_CONST_VIRTUAL_CALL(base, methodName, methodParams) \
    switch (astNodeType) \
    { \
        SLANG_ALL_ASTNode_##base(SLANG_AST_NODE_CONST_CASE, (methodName, methodParams)) \
        default: return SLANG_AST_NODE_INVOKE (methodName, methodParams); \
    }

#endif // SLANG_AST_REFLECT_H
