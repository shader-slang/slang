// slang-ast-decl.cpp
#include "slang-ast-builder.h"
#include <assert.h>

#include "slang-generated-ast-macro.h"

namespace Slang {

const TypeExp& TypeConstraintDecl::getSup() const
{
    SLANG_AST_NODE_CONST_VIRTUAL_CALL(TypeConstraintDecl, getSup, ())
}

const TypeExp& TypeConstraintDecl::_getSupOverride() const
{
    SLANG_UNEXPECTED("TypeConstraintDecl::_getSupOverride not overridden");
    //return TypeExp::empty;
}


} // namespace Slang
