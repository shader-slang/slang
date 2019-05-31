#ifndef SLANG_REFLECTION_H
#define SLANG_REFLECTION_H

#include "../core/slang-basic.h"
#include "slang-syntax.h"

#include "../../slang.h"

namespace Slang {

class ProgramLayout;
class TypeLayout;

//

SlangTypeKind getReflectionTypeKind(Type* type);

SlangTypeKind getReflectionParameterCategory(TypeLayout* typeLayout);

UInt getReflectionFieldCount(Type* type);
UInt getReflectionFieldByIndex(Type* type, UInt index);
UInt getReflectionFieldByIndex(TypeLayout* typeLayout, UInt index);

}

#endif // SLANG_REFLECTION_H
