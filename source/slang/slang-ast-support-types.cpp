#include "slang-ast-support-types.h"
#include "slang-ast-base.h"
#include "slang-ast-type.h"

Slang::QualType::QualType(Type* type)
    : type(type)
    , isLeftValue(false)
{
    if (as<RefType>(type))
    {
        isLeftValue = true;
    }
}
