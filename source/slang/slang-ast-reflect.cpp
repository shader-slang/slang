#include "../../slang.h"

#include "slang-ast-reflect.h"

#include "../core/slang-smart-pointer.h"

#include "slang-ast-all.h"

#include <typeinfo>
#include <assert.h>

#include "slang-visitor.h"

#include "slang-ast-generated-macro.h"

namespace Slang
{

#define SLANG_REFLECT_GET_REFLECT_CLASS_INFO(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) infos.infos[int(ASTNodeType::NAME)] = &NAME::kReflectClassInfo;

static ReflectClassInfo::Infos _calcInfos()
{
    ReflectClassInfo::Infos infos;
    memset(&infos, 0, sizeof(infos));
    SLANG_ALL_ASTNode_NodeBase(SLANG_REFLECT_GET_REFLECT_CLASS_INFO, _)
    SLANG_ALL_ASTNode_Substitutions(SLANG_REFLECT_GET_REFLECT_CLASS_INFO, _)
    return infos;
}

/* static */const ReflectClassInfo::Infos ReflectClassInfo::kInfos = _calcInfos();

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

// Now try and implement all of the classes
// Macro generated is of the format


template <typename T>
struct CreateImpl
{
    static void* create() { return new T; }
};

#define SLANG_GET_SUPER_BASE(SUPER) nullptr
#define SLANG_GET_SUPER_INNER(SUPER) &SUPER::kReflectClassInfo
#define SLANG_GET_SUPER_LEAF(SUPER) &SUPER::kReflectClassInfo

#define SLANG_GET_CREATE_FUNC_ABSTRACT(NAME) nullptr
#define SLANG_GET_CREATE_FUNC_NONE(NAME) &CreateImpl<NAME>::create

#define SLANG_GET_CREATE_FUNC_NON_VISITOR_ABSTRACT(NAME) nullptr
#define SLANG_GET_CREATE_FUNC_NON_VISITOR(NAME) &CreateImpl<NAME>::create


#define SLANG_REFLECT_CLASS_INFO(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \
    /* static */const ReflectClassInfo NAME::kReflectClassInfo = { uint32_t(ASTNodeType::NAME), uint32_t(ASTNodeType::LAST), SLANG_GET_SUPER_##TYPE(SUPER), #NAME, SLANG_GET_CREATE_FUNC_##MARKER(NAME)  };

SLANG_ALL_ASTNode_NodeBase(SLANG_REFLECT_CLASS_INFO, _)
SLANG_ALL_ASTNode_Substitutions(SLANG_REFLECT_CLASS_INFO, _)

// We dispatch to non 'abstract' types
#define SLANG_CASE_NONE(NAME)           case ASTNodeType::NAME: return visitor->dispatch_##NAME(static_cast<NAME*>(this), extra);
#define SLANG_CASE_ABSTRACT(NAME)

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
