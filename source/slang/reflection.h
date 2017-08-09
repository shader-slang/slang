#ifndef SLANG_REFLECTION_H
#define SLANG_REFLECTION_H

#include "../core/basic.h"
#include "syntax.h"

#include "../../slang.h"

namespace Slang {

// TODO(tfoley): Need to move these somewhere universal

typedef intptr_t    Int;
typedef int64_t     Int64;

typedef uintptr_t   UInt;
typedef uint64_t    UInt64;

class ProgramLayout;
class TypeLayout;

String emitReflectionJSON(
    ProgramLayout* programLayout);

//

SlangTypeKind getReflectionTypeKind(Type* type);

SlangTypeKind getReflectionParameterCategory(TypeLayout* typeLayout);

UInt getReflectionFieldCount(Type* type);
UInt getReflectionFieldByIndex(Type* type, UInt index);
UInt getReflectionFieldByIndex(TypeLayout* typeLayout, UInt index);

}

#endif // SLANG_REFLECTION_H
