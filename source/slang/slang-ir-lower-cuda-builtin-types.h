#ifndef SLANG_IR_LOWER_BUILTIN_TYPES_H
#define SLANG_IR_LOWER_BUILTIN_TYPES_H

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-clone.h"
#include "slang-ir-layout.h"

namespace Slang
{
    struct LoweredBuiltinTypeInfo
    {
        IRType* originalType;
        IRType* loweredType;
        IRType* loweredInnerArrayType = nullptr; // For matrix/array types that are lowered into a struct type, this is the inner array type of the data field.
        IRStructKey* loweredInnerStructKey = nullptr; // For matrix/array types that are lowered into a struct type, this is the struct key of the data field.
        IRFunc* convertOriginalToLowered = nullptr;
        IRFunc* convertLoweredToOriginal = nullptr;
    };

    LoweredBuiltinTypeInfo lowerMatrixType(
        IRBuilder* builder,
        IRMatrixType* matrixType,
        String nameSuffix = "");

    LoweredBuiltinTypeInfo lowerVectorType(
        IRBuilder* builder,
        IRVectorType* vectorType,
        String nameSuffix = "");

} // namespace Slang

#endif // SLANG_IR_LOWER_BUILTIN_TYPES_H