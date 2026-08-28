#pragma once

#include "slang-nvvm-ir-builder-api.h"

namespace Slang
{
namespace NVVMSemantics
{

/// Identifies the provider lowering family and, where present, its frozen V3 adapter family.
enum class LegacyFamily : uint32_t
{
    IntegerUnary,
    IntegerBinary,
    IntegerCompare,
    FloatingUnary,
    FloatingBinary,
    FloatingCompare,
    Intrinsic,
    V4ExecutionRegister,
    V4WorkgroupBarrier,
};

/// Identifies a parameterized V4-only operation family outside the frozen compatibility table.
enum class V4Family : uint32_t
{
    None,
    IntegerUnary,
    IntegerBinary,
    IntegerCompare,
    IntegerConvert,
    IntegerToFloat,
    FloatToInteger,
    SignedI32x2Add,
};

struct V4FamilyResolution
{
    V4Family family = V4Family::None;
    const char* diagnosticName = nullptr;
};

/// Describes one established semantic overload from canonical Slang values to the provider ABI.
struct CatalogEntry
{
    SlangNVVMValueOperation operation;
    SlangNVVMValueTypeDesc resultType;
    SlangNVVMValueTypeDesc operandTypes[3];
    uint32_t operandCount;
    SlangNVVMBuilderFeature legacyFeature;
    LegacyFamily legacyFamily;
    uint32_t legacyOperation;
    const char* diagnosticName;
    const char* genericAsm;
};

inline constexpr SlangNVVMValueTypeDesc kNoType = {};
inline constexpr SlangNVVMValueTypeDesc kVoid = {
    SLANG_NVVM_VALUE_TYPE_VOID,
    0,
    0,
};
inline constexpr SlangNVVMValueTypeDesc kBool = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, 1};
inline constexpr SlangNVVMValueTypeDesc kSignedI32 = {
    SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
    32,
    1,
};
inline constexpr SlangNVVMValueTypeDesc kUnsignedI32 = {
    SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    32,
    1,
};
inline constexpr SlangNVVMValueTypeDesc kUnsignedI32x3 = {
    SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    32,
    3,
};
inline constexpr SlangNVVMValueTypeDesc kFloat32 = {
    SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
    32,
    1,
};
inline constexpr SlangNVVMValueTypeDesc kSignedI32x2 = {
    SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
    32,
    2,
};

// This is the only table that maps an established typed semantic to its provider operation and,
// where one exists, its frozen V3 compatibility operation. GenericAsm spellings are present only
// for semantics produced through that canonical CUDA helper shape; ordinary IR operations select
// the same typed rows without a spelling.
inline constexpr CatalogEntry kCatalog[] = {
    {
        SLANG_NVVM_VALUE_OP_ADD,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        "signed i32 addition",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_SUBTRACT,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_SUBTRACT,
        "signed i32 subtraction",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_MULTIPLY,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_MULTIPLY,
        "signed i32 multiplication",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_AND,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_AND,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_BIT_AND,
        "signed i32 bitwise AND",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_OR,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_OR,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_BIT_OR,
        "signed i32 bitwise OR",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_XOR,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_XOR,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_BIT_XOR,
        "signed i32 bitwise XOR",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_NOT,
        kSignedI32,
        {kSignedI32, kNoType, kNoType},
        1,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_NOT,
        LegacyFamily::IntegerUnary,
        SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT,
        "signed i32 bitwise NOT",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_NEGATE,
        kSignedI32,
        {kSignedI32, kNoType, kNoType},
        1,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NEGATE,
        LegacyFamily::IntegerUnary,
        SLANG_NVVM_INTEGER_UNARY_OP_NEGATE,
        "signed i32 arithmetic negation",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_EQUAL,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_EQUAL,
        LegacyFamily::IntegerCompare,
        SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL,
        "signed i32 equality comparison",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_NOT_EQUAL,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_NOT_EQUAL,
        LegacyFamily::IntegerCompare,
        SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL,
        "signed i32 inequality comparison",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_LESS_THAN,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW,
        LegacyFamily::IntegerCompare,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN,
        "signed i32 less-than comparison",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_GREATER_THAN,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_THAN,
        LegacyFamily::IntegerCompare,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN,
        "signed i32 greater-than comparison",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_LESS_EQUAL,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_LESS_EQUAL,
        LegacyFamily::IntegerCompare,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL,
        "signed i32 less-than-or-equal comparison",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_GREATER_EQUAL,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_SIGNED_GREATER_EQUAL,
        LegacyFamily::IntegerCompare,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL,
        "signed i32 greater-than-or-equal comparison",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_ADD,
        kFloat32,
        {kFloat32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD,
        LegacyFamily::FloatingBinary,
        SLANG_NVVM_FLOATING_BINARY_OP_ADD,
        "float32 addition",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_SUBTRACT,
        kFloat32,
        {kFloat32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT,
        LegacyFamily::FloatingBinary,
        SLANG_NVVM_FLOATING_BINARY_OP_SUBTRACT,
        "float32 subtraction",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_MULTIPLY,
        kFloat32,
        {kFloat32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_MULTIPLY,
        LegacyFamily::FloatingBinary,
        SLANG_NVVM_FLOATING_BINARY_OP_MULTIPLY,
        "float32 multiplication",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_DIVIDE,
        kFloat32,
        {kFloat32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_DIVIDE,
        LegacyFamily::FloatingBinary,
        SLANG_NVVM_FLOATING_BINARY_OP_DIVIDE,
        "float32 division",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_NEGATE,
        kFloat32,
        {kFloat32, kNoType, kNoType},
        1,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NEGATE,
        LegacyFamily::FloatingUnary,
        SLANG_NVVM_FLOATING_UNARY_OP_NEGATE,
        "float32 negation",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_EQUAL,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_EQUAL,
        LegacyFamily::FloatingCompare,
        SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_EQUAL,
        "float32 ordered equality",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_NOT_EQUAL,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_NOT_EQUAL,
        LegacyFamily::FloatingCompare,
        SLANG_NVVM_FLOATING_COMPARE_OP_UNORDERED_NOT_EQUAL,
        "float32 unordered inequality",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_LESS_THAN,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_THAN,
        LegacyFamily::FloatingCompare,
        SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_LESS_THAN,
        "float32 ordered less-than",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_GREATER_THAN,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_THAN,
        LegacyFamily::FloatingCompare,
        SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_GREATER_THAN,
        "float32 ordered greater-than",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_LESS_EQUAL,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_LESS_EQUAL,
        LegacyFamily::FloatingCompare,
        SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_LESS_EQUAL,
        "float32 ordered less-than-or-equal",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_GREATER_EQUAL,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ORDERED_GREATER_EQUAL,
        LegacyFamily::FloatingCompare,
        SLANG_NVVM_FLOATING_COMPARE_OP_ORDERED_GREATER_EQUAL,
        "float32 ordered greater-than-or-equal",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX,
        kUnsignedI32,
        {kNoType, kNoType, kNoType},
        0,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_INDEX,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_INDEX,
        "wave lane index intrinsic",
        "_getLaneId()",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_LANE_COUNT,
        kUnsignedI32,
        {kNoType, kNoType, kNoType},
        0,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_LANE_COUNT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_LANE_COUNT,
        "wave lane count intrinsic",
        "(warpSize)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kUnsignedI32,
        {kUnsignedI32, kUnsignedI32, kSignedI32},
        3,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_UINT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_UINT,
        "UInt wave read-lane-at intrinsic",
        "__shfl_sync($0, $1, $2)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kSignedI32,
        {kUnsignedI32, kSignedI32, kSignedI32},
        3,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_INT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_INT,
        "Int wave read-lane-at intrinsic",
        "__shfl_sync($0, $1, $2)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kFloat32,
        {kUnsignedI32, kFloat32, kSignedI32},
        3,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_AT_FLOAT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_AT_FLOAT,
        "Float wave read-lane-at intrinsic",
        "__shfl_sync($0, $1, $2)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT,
        kUnsignedI32,
        {kUnsignedI32, kBool, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_BALLOT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_BALLOT,
        "wave-mask ballot intrinsic",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        kUnsignedI32,
        {kUnsignedI32, kUnsignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_UINT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_UINT,
        "UInt wave read-lane-first intrinsic",
        "_waveReadFirst($0, $1)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        kSignedI32,
        {kUnsignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_INT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_INT,
        "Int wave read-lane-first intrinsic",
        "_waveReadFirst($0, $1)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        kFloat32,
        {kUnsignedI32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_READ_LANE_FIRST_FLOAT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_READ_LANE_FIRST_FLOAT,
        "Float wave read-lane-first intrinsic",
        "_waveReadFirst($0, $1)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_IS_FIRST_LANE,
        kBool,
        {kUnsignedI32, kNoType, kNoType},
        1,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_IS_FIRST_LANE,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_IS_FIRST_LANE,
        "wave-mask is-first-lane intrinsic",
        "(($0 & -$0) == (WarpMask(1) << _getLaneId()))",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ANY_TRUE,
        kBool,
        {kUnsignedI32, kBool, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ANY_TRUE,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ANY_TRUE,
        "wave-mask any-true intrinsic",
        "(__any_sync($0, $1) != 0)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_TRUE,
        kBool,
        {kUnsignedI32, kBool, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_TRUE,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_TRUE,
        "wave-mask all-true intrinsic",
        "(__all_sync($0, $1) != 0)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        kBool,
        {kUnsignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_INT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_INT,
        "signed-i32 wave-mask all-equal intrinsic",
        "_waveAllEqual($0, $1)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        kBool,
        {kUnsignedI32, kUnsignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_UINT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_UINT,
        "unsigned-i32 wave-mask all-equal intrinsic",
        "_waveAllEqual($0, $1)",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        kBool,
        {kUnsignedI32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_FLOAT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_FLOAT,
        "float32 wave-mask all-equal intrinsic",
        "_waveAllEqual($0, $1)",
    },
    {
        SLANG_NVVM_VALUE_OP_THREAD_INDEX,
        kUnsignedI32x3,
        {kNoType, kNoType, kNoType},
        0,
        SLANG_NVVM_BUILDER_FEATURE_COUNT,
        LegacyFamily::V4ExecutionRegister,
        0,
        "CUDA thread index",
        "(threadIdx)",
    },
    {
        SLANG_NVVM_VALUE_OP_BLOCK_INDEX,
        kUnsignedI32x3,
        {kNoType, kNoType, kNoType},
        0,
        SLANG_NVVM_BUILDER_FEATURE_COUNT,
        LegacyFamily::V4ExecutionRegister,
        0,
        "CUDA block index",
        "(blockIdx)",
    },
    {
        SLANG_NVVM_VALUE_OP_BLOCK_DIMENSIONS,
        kUnsignedI32x3,
        {kNoType, kNoType, kNoType},
        0,
        SLANG_NVVM_BUILDER_FEATURE_COUNT,
        LegacyFamily::V4ExecutionRegister,
        0,
        "CUDA block dimensions",
        "(blockDim)",
    },
    {
        SLANG_NVVM_VALUE_OP_GRID_DIMENSIONS,
        kUnsignedI32x3,
        {kNoType, kNoType, kNoType},
        0,
        SLANG_NVVM_BUILDER_FEATURE_COUNT,
        LegacyFamily::V4ExecutionRegister,
        0,
        "CUDA grid dimensions",
        "(gridDim)",
    },
    {
        SLANG_NVVM_VALUE_OP_WORKGROUP_BARRIER,
        kVoid,
        {kNoType, kNoType, kNoType},
        0,
        SLANG_NVVM_BUILDER_FEATURE_COUNT,
        LegacyFamily::V4WorkgroupBarrier,
        0,
        "CUDA workgroup barrier",
        "__syncthreads()",
    },
};

inline constexpr size_t getCatalogCount()
{
    return sizeof(kCatalog) / sizeof(kCatalog[0]);
}

inline bool areSameType(const SlangNVVMValueTypeDesc& left, const SlangNVVMValueTypeDesc& right)
{
    return left.kind == right.kind && left.bitWidth == right.bitWidth &&
           left.laneCount == right.laneCount;
}

inline SlangNVVMValueOperationDesc getOperationDesc(const CatalogEntry& entry)
{
    return {
        entry.operation,
        entry.resultType,
        entry.operandCount ? entry.operandTypes : nullptr,
        entry.operandCount,
    };
}

inline bool hasLegacyAdapter(const CatalogEntry& entry)
{
    return entry.legacyFeature < SLANG_NVVM_BUILDER_FEATURE_COUNT;
}

/// Returns the unique established row behind one frozen V3 compatibility operation.
inline const CatalogEntry* findLegacyOperation(LegacyFamily family, uint32_t operation)
{
    for (const CatalogEntry& entry : kCatalog)
    {
        if (hasLegacyAdapter(entry) && entry.legacyFamily == family &&
            entry.legacyOperation == operation)
            return &entry;
    }
    return nullptr;
}

/// Returns the one established catalog row that exactly matches a complete typed operation.
inline const CatalogEntry* find(const SlangNVVMValueOperationDesc& desc)
{
    if (!desc.operandTypes && desc.operandCount)
        return nullptr;

    for (const CatalogEntry& entry : kCatalog)
    {
        if (entry.operation != desc.operation || entry.operandCount != desc.operandCount ||
            !areSameType(entry.resultType, desc.resultType))
        {
            continue;
        }
        bool operandsMatch = true;
        for (uint32_t i = 0; i < entry.operandCount; ++i)
            operandsMatch =
                operandsMatch && areSameType(entry.operandTypes[i], desc.operandTypes[i]);
        if (operandsMatch)
            return &entry;
    }
    return nullptr;
}

inline bool isSelectedScalarInteger(const SlangNVVMValueTypeDesc& type)
{
    const bool isInteger = type.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                           type.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER;
    const bool isSelectedWidth =
        type.bitWidth == 8 || type.bitWidth == 16 || type.bitWidth == 32 || type.bitWidth == 64;
    return isInteger && isSelectedWidth && type.laneCount == 1;
}

/// Resolves the bounded, dimensioned numeric families added after the frozen exact catalog.
inline bool resolveV4Family(
    const SlangNVVMValueOperationDesc& desc,
    V4FamilyResolution& outResolution)
{
    outResolution = {};
    if (!desc.operandTypes && desc.operandCount)
        return false;

    const bool isUnaryInteger = desc.operandCount == 1 &&
                                isSelectedScalarInteger(desc.resultType) &&
                                areSameType(desc.resultType, desc.operandTypes[0]);
    if (isUnaryInteger && (desc.operation == SLANG_NVVM_VALUE_OP_BIT_NOT ||
                           desc.operation == SLANG_NVVM_VALUE_OP_NEGATE))
    {
        outResolution = {V4Family::IntegerUnary, "parameterized integer unary operation"};
        return true;
    }

    const bool isBinaryInteger = desc.operandCount == 2 &&
                                 isSelectedScalarInteger(desc.resultType) &&
                                 areSameType(desc.resultType, desc.operandTypes[0]) &&
                                 areSameType(desc.resultType, desc.operandTypes[1]);
    if (isBinaryInteger && (desc.operation == SLANG_NVVM_VALUE_OP_ADD ||
                            desc.operation == SLANG_NVVM_VALUE_OP_SUBTRACT ||
                            desc.operation == SLANG_NVVM_VALUE_OP_MULTIPLY ||
                            desc.operation == SLANG_NVVM_VALUE_OP_BIT_AND ||
                            desc.operation == SLANG_NVVM_VALUE_OP_BIT_OR ||
                            desc.operation == SLANG_NVVM_VALUE_OP_BIT_XOR))
    {
        outResolution = {V4Family::IntegerBinary, "parameterized integer binary operation"};
        return true;
    }

    const bool isIntegerCompare = desc.operandCount == 2 && areSameType(desc.resultType, kBool) &&
                                  isSelectedScalarInteger(desc.operandTypes[0]) &&
                                  areSameType(desc.operandTypes[0], desc.operandTypes[1]);
    if (isIntegerCompare && desc.operation >= SLANG_NVVM_VALUE_OP_EQUAL &&
        desc.operation <= SLANG_NVVM_VALUE_OP_GREATER_EQUAL)
    {
        outResolution = {V4Family::IntegerCompare, "parameterized integer comparison"};
        return true;
    }

    if (desc.operation == SLANG_NVVM_VALUE_OP_INTEGER_CONVERT && desc.operandCount == 1 &&
        isSelectedScalarInteger(desc.resultType) && isSelectedScalarInteger(desc.operandTypes[0]) &&
        !areSameType(desc.resultType, desc.operandTypes[0]))
    {
        outResolution = {V4Family::IntegerConvert, "explicit integer conversion"};
        return true;
    }
    if (desc.operation == SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT && desc.operandCount == 1 &&
        areSameType(desc.resultType, kFloat32) && isSelectedScalarInteger(desc.operandTypes[0]))
    {
        outResolution = {V4Family::IntegerToFloat, "integer-to-float32 conversion"};
        return true;
    }
    if (desc.operation == SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER && desc.operandCount == 1 &&
        isSelectedScalarInteger(desc.resultType) && areSameType(desc.operandTypes[0], kFloat32))
    {
        outResolution = {V4Family::FloatToInteger, "float32-to-integer conversion"};
        return true;
    }

    if (desc.operation == SLANG_NVVM_VALUE_OP_ADD && desc.operandCount == 2 &&
        areSameType(desc.resultType, kSignedI32x2) &&
        areSameType(desc.operandTypes[0], kSignedI32x2) &&
        areSameType(desc.operandTypes[1], kSignedI32x2))
    {
        outResolution = {V4Family::SignedI32x2Add, "signed i32x2 addition"};
        return true;
    }
    return false;
}

inline bool isSupported(const SlangNVVMValueOperationDesc& desc)
{
    V4FamilyResolution resolution;
    return find(desc) || resolveV4Family(desc, resolution);
}

} // namespace NVVMSemantics
} // namespace Slang
