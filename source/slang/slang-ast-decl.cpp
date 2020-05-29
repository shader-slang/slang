// slang-ast-decl.cpp
#include "slang-ast-builder.h"
#include <assert.h>

namespace Slang {

const TypeExp& TypeConstraintDecl::getSup() const
{
    switch (astNodeType)
    {
#define SLANG_CASE(NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param)      case ASTNodeType::NAME: return static_cast<const NAME*>(this)->_getSupOverride();
        SLANG_ASTNode_TypeConstraintDecl(SLANG_CASE, _)
#undef SLANG_CASE
        default: break;
    }
    SLANG_ASSERT(!"getSup not implemented for this type!");
    return TypeExp::empty;
}

} // namespace Slang
