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

// A special case used in Val::accept, because can be Type derived too
#define SLANG_CASE_TYPE_NONE(NAME)           case ASTNodeType::NAME: return ((ITypeVisitor*)visitor)->dispatch_##NAME(static_cast<NAME*>(this), extra);
#define SLANG_CASE_TYPE_ABSTRACT(NAME)

#define SLANG_CASE_TYPE_DISPATCH(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param)  SLANG_CASE_TYPE_##MARKER(NAME)

// Use origin lists, to generate regular and type dispatch

#define SLANG_ORIGIN_CASE_DISPATCH(NAME, param) SLANG_ASTNode_##NAME(SLANG_CASE_DISPATCH, param)
#define SLANG_ORIGIN_CASE_TYPE_DISPATCH(NAME, param) SLANG_ASTNode_##NAME(SLANG_CASE_TYPE_DISPATCH, param)

void Val::accept(IValVisitor* visitor, void* extra)
{
    const ReflectClassInfo& classInfo = getClassInfo();
    const ASTNodeType astType = ASTNodeType(classInfo.m_classId);

    switch (astType)
    {
        // Here some trickery is used. When virtual methods were used, the override of the IValVistor
        // function on Type, would cast to a ITypeVisitor and then 'accept' that.
        //
        // Here I want to dispatch to the correct visitor depending on the underlying type.
        // A problem is that the 'Val' hierachy contains both Type and other things, so if I want a single
        // switch I have to partition.
        //
        // A simpler (but less efficient) method would be to cast this to a type, and if that works use the ITypeVistor.
        // Here I use the 'ORIGIN' which uses the file 'ORIGIN' of the class. It turns out that Type and Val are partitioned
        // such that there is no cross over, and contain the appropriate split of types
        SLANG_ORIGIN_TYPE_ASTNode(SLANG_ORIGIN_CASE_TYPE_DISPATCH, _)
        SLANG_ORIGIN_VAL_ASTNode(SLANG_ORIGIN_CASE_DISPATCH, _)

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
