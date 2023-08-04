#include "../../slang.h"

#include "slang-ast-reflect.h"

#include "../core/slang-smart-pointer.h"

#include "slang-ast-all.h"

#include <typeinfo>
#include <assert.h>

#include "slang-visitor.h"

#include "slang-generated-ast-macro.h"

namespace Slang
{

#define SLANG_REFLECT_GET_REFLECT_CLASS_INFO(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) infos.infos[int(ASTNodeType::NAME)] = &NAME::kReflectClassInfo;

static ASTClassInfo::Infos _calcInfos()
{
    ASTClassInfo::Infos infos;
    memset(&infos, 0, sizeof(infos));
    SLANG_ALL_ASTNode_NodeBase(SLANG_REFLECT_GET_REFLECT_CLASS_INFO, _)
    return infos;
}

/* static */const ASTClassInfo::Infos ASTClassInfo::kInfos = _calcInfos();

// Now try and implement all of the classes
// Macro generated is of the format

struct ASTConstructAccess
{
    template <typename T>
    struct Impl
    {
        static void* create(void* context)
        {
            ASTBuilder* astBuilder = (ASTBuilder*)context;
            return astBuilder->createImpl<T>();
        }
        static void destroy(void* ptr)
        {
            // Needed because if type has non dtor, Visual Studio claims ptr not used
            SLANG_UNUSED(ptr);
            reinterpret_cast<T*>(ptr)->~T();
        }
    };
};

#define SLANG_GET_SUPER_BASE(SUPER) nullptr
#define SLANG_GET_SUPER_INNER(SUPER) &SUPER::kReflectClassInfo
#define SLANG_GET_SUPER_LEAF(SUPER) &SUPER::kReflectClassInfo

#define SLANG_GET_CREATE_FUNC_ABSTRACT_AST(NAME) nullptr
#define SLANG_GET_CREATE_FUNC_AST(NAME) &ASTConstructAccess::Impl<NAME>::create

#define SLANG_GET_DESTROY_FUNC_ABSTRACT_AST(NAME) nullptr
#define SLANG_GET_DESTROY_FUNC_AST(NAME) &ASTConstructAccess::Impl<NAME>::destroy

#define SLANG_REFLECT_CLASS_INFO(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    /* static */const ReflectClassInfo NAME::kReflectClassInfo = { uint32_t(ASTNodeType::NAME), uint32_t(ASTNodeType::LAST), SLANG_GET_SUPER_##TYPE(SUPER), #NAME, SLANG_GET_CREATE_FUNC_##MARKER(NAME), SLANG_GET_DESTROY_FUNC_##MARKER(NAME), uint32_t(sizeof(NAME)), uint8_t(SLANG_ALIGN_OF(NAME)) };

SLANG_ALL_ASTNode_NodeBase(SLANG_REFLECT_CLASS_INFO, _)

// We dispatch to non 'abstract' types
#define SLANG_CASE_AST(NAME)           case ASTNodeType::NAME: return visitor->dispatch_##NAME(static_cast<NAME*>(this), extra);
#define SLANG_CASE_ABSTRACT_AST(NAME)

#define SLANG_CASE_DISPATCH(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param)  SLANG_CASE_##MARKER(NAME)

void Val::accept(IValVisitor* visitor, void* extra)
{
    const ReflectClassInfo& classInfo = getClassInfo();
    const ASTNodeType astType = ASTNodeType(classInfo.m_classId);

    switch (astType)
    {
        SLANG_CHILDREN_ASTNode_Val(SLANG_CASE_DISPATCH, _)
        default: SLANG_ASSERT(!"Unknown type");
    }
}

void Type::accept(ITypeVisitor* visitor, void* extra)
{
    const ReflectClassInfo& classInfo = getClassInfo();
    const ASTNodeType astType = ASTNodeType(classInfo.m_classId);

    switch (astType)
    {
        SLANG_CHILDREN_ASTNode_Type(SLANG_CASE_DISPATCH, _)
        default: SLANG_ASSERT(!"Unknown type");
    }
}

void Modifier::accept(IModifierVisitor* visitor, void* extra)
{
    const ReflectClassInfo& classInfo = getClassInfo();
    const ASTNodeType astType = ASTNodeType(classInfo.m_classId);

    switch (astType)
    {
        SLANG_CHILDREN_ASTNode_Modifier(SLANG_CASE_DISPATCH, _)
        default: SLANG_ASSERT(!"Unknown type");
    }
}

void DeclBase::accept(IDeclVisitor* visitor, void* extra)
{
    const ReflectClassInfo& classInfo = getClassInfo();
    const ASTNodeType astType = ASTNodeType(classInfo.m_classId);

    switch (astType)
    {
        SLANG_CHILDREN_ASTNode_DeclBase(SLANG_CASE_DISPATCH, _)
        default: SLANG_ASSERT(!"Unknown type");
    }
}

void Expr::accept(IExprVisitor* visitor, void* extra)
{
    const ReflectClassInfo& classInfo = getClassInfo();
    const ASTNodeType astType = ASTNodeType(classInfo.m_classId);

    switch (astType)
    {
        SLANG_CHILDREN_ASTNode_Expr(SLANG_CASE_DISPATCH, _)
        default: SLANG_ASSERT(!"Unknown type");
    }
}

void Stmt::accept(IStmtVisitor* visitor, void* extra)
{
    const ReflectClassInfo& classInfo = getClassInfo();
    const ASTNodeType astType = ASTNodeType(classInfo.m_classId);

    switch (astType)
    {
        SLANG_CHILDREN_ASTNode_Stmt(SLANG_CASE_DISPATCH, _)
        default: SLANG_ASSERT(!"Unknown type");
    }
}

} // namespace Slang
