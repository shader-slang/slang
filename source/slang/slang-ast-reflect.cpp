#include "../../slang.h"

#include "slang-ast-reflect.h"

#include "../core/slang-smart-pointer.h"

#include "slang-ast-all.h"

#include <typeinfo>
#include <assert.h>

#include "slang-ast-generated-macro.h"

namespace Slang
{

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

#define SLANG_GET_SUPER_BASE(NAME) nullptr
#define SLANG_GET_SUPER_INNER(NAME) &ASTNodeSuper::NAME##_Super::kReflectClassInfo
#define SLANG_GET_SUPER_LEAF(NAME) &ASTNodeSuper::NAME##_Super::kReflectClassInfo

#define SLANG_GET_CREATE_FUNC_ABSTRACT(NAME) nullptr
#define SLANG_GET_CREATE_FUNC_NORMAL(NAME) &CreateImpl<NAME>::create

#define SLANG_GET_CREATE_FUNC_NON_VISITOR_ABSTRACT(NAME) nullptr
#define SLANG_GET_CREATE_FUNC_NON_VISITOR(NAME) &CreateImpl<NAME>::create


#define SLANG_REFLECT_CLASS_INFO(NAME, SUPER, STYLE, NODE_STYLE, PARAM) \
    /* static */const ReflectClassInfo NAME::kReflectClassInfo = { uint32_t(ASTNodeType::NAME), uint32_t(ASTNodeLast::NAME), SLANG_GET_SUPER_##NODE_STYLE(NAME), #NAME, SLANG_GET_CREATE_FUNC_##STYLE(NAME)  };

SLANG_ASTNode_NodeBase(SLANG_REFLECT_CLASS_INFO, _)
SLANG_ASTNode_Substitutions(SLANG_REFLECT_CLASS_INFO, _)

#if 0
#define SLANG_DISPATCH(name, abstractStyle, nodeStyle, param) \
    virtual void dispatch_##name(name* obj, void* extra) = 0;

struct ITypeVisitor
{
    // All of the types that are derived from Type
    SLANG_DERIVED_ASTNode_Type(SLANG_DISPATCH, _)
};

#endif

} // namespace Slang
