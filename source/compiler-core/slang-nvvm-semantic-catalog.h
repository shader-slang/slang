#pragma once

#include "slang-nvvm-ir-builder-api.h"

namespace Slang
{
namespace NVVMSemantics
{

/// Identifies the frozen V3 callback family used only by the compatibility adapter.
enum class LegacyFamily : uint32_t
{
    IntegerUnary,
    IntegerBinary,
    IntegerCompare,
    FloatingUnary,
    FloatingBinary,
    FloatingCompare,
    Intrinsic,
};

/// Describes one established semantic overload from canonical Slang values to the provider ABI.
struct CatalogEntry
{
    SlangNVVMValueOperation_4 operation;
    SlangNVVMValueTypeDesc_4 resultType;
    SlangNVVMValueTypeDesc_4 operandTypes[3];
    uint32_t operandCount;
    SlangNVVMBuilderFeature_3 legacyFeature;
    LegacyFamily legacyFamily;
    uint32_t legacyOperation;
    const char* diagnosticName;
    const char* genericAsm;
};

inline constexpr SlangNVVMValueTypeDesc_4 kNoType = {};
inline constexpr SlangNVVMValueTypeDesc_4 kBool = {SLANG_NVVM_VALUE_TYPE_BOOL_4, 1, 1, 0};
inline constexpr SlangNVVMValueTypeDesc_4 kSignedI32 = {
    SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER_4,
    32,
    1,
    0,
};
inline constexpr SlangNVVMValueTypeDesc_4 kUnsignedI32 = {
    SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER_4,
    32,
    1,
    0,
};
inline constexpr SlangNVVMValueTypeDesc_4 kFloat32 = {
    SLANG_NVVM_VALUE_TYPE_FLOATING_POINT_4,
    32,
    1,
    0,
};

// This is the only table that maps an established typed semantic to its frozen V3 compatibility
// operation. GenericAsm spellings are present only for semantics produced through that canonical
// CUDA helper shape; ordinary IR operations select the same typed rows without a spelling.
inline constexpr CatalogEntry kCatalog[] = {
    {
        SLANG_NVVM_VALUE_OP_ADD_4,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_3_ADD,
        "signed i32 addition",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_SUBTRACT_4,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_CONTROL_FLOW,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_3_SUB,
        "signed i32 subtraction",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_MULTIPLY_4,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_3_MULTIPLY,
        "signed i32 multiplication",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_AND_4,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_AND,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_AND,
        "signed i32 bitwise AND",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_OR_4,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_OR,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_OR,
        "signed i32 bitwise OR",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_XOR_4,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_BIT_XOR,
        LegacyFamily::IntegerBinary,
        SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_XOR,
        "signed i32 bitwise XOR",
        nullptr,
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_NOT_4,
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
        SLANG_NVVM_VALUE_OP_NEGATE_4,
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
        SLANG_NVVM_VALUE_OP_EQUAL_4,
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
        SLANG_NVVM_VALUE_OP_NOT_EQUAL_4,
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
        SLANG_NVVM_VALUE_OP_LESS_THAN_4,
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
        SLANG_NVVM_VALUE_OP_GREATER_THAN_4,
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
        SLANG_NVVM_VALUE_OP_LESS_EQUAL_4,
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
        SLANG_NVVM_VALUE_OP_GREATER_EQUAL_4,
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
        SLANG_NVVM_VALUE_OP_ADD_4,
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
        SLANG_NVVM_VALUE_OP_SUBTRACT_4,
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
        SLANG_NVVM_VALUE_OP_MULTIPLY_4,
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
        SLANG_NVVM_VALUE_OP_DIVIDE_4,
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
        SLANG_NVVM_VALUE_OP_NEGATE_4,
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
        SLANG_NVVM_VALUE_OP_EQUAL_4,
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
        SLANG_NVVM_VALUE_OP_NOT_EQUAL_4,
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
        SLANG_NVVM_VALUE_OP_LESS_THAN_4,
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
        SLANG_NVVM_VALUE_OP_GREATER_THAN_4,
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
        SLANG_NVVM_VALUE_OP_LESS_EQUAL_4,
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
        SLANG_NVVM_VALUE_OP_GREATER_EQUAL_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_LANE_COUNT_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_MASK_IS_FIRST_LANE_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ANY_TRUE_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_TRUE_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL_4,
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
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL_4,
        kBool,
        {kUnsignedI32, kFloat32, kNoType},
        2,
        SLANG_NVVM_BUILDER_FEATURE_WAVE_MASK_ALL_EQUAL_FLOAT,
        LegacyFamily::Intrinsic,
        SLANG_NVVM_INTRINSIC_OP_WAVE_MASK_ALL_EQUAL_FLOAT,
        "float32 wave-mask all-equal intrinsic",
        "_waveAllEqual($0, $1)",
    },
};

inline constexpr size_t getCatalogCount()
{
    return sizeof(kCatalog) / sizeof(kCatalog[0]);
}

inline bool areSameType(const SlangNVVMValueTypeDesc_4& left, const SlangNVVMValueTypeDesc_4& right)
{
    return left.kind == right.kind && left.bitWidth == right.bitWidth &&
           left.laneCount == right.laneCount && left.reserved == right.reserved;
}

inline SlangNVVMValueOperationDesc_4 getOperationDesc(const CatalogEntry& entry)
{
    return {
        uint32_t(sizeof(SlangNVVMValueOperationDesc_4)),
        entry.operation,
        entry.resultType,
        entry.operandCount ? entry.operandTypes : nullptr,
        entry.operandCount,
    };
}

/// Returns the unique established row behind one frozen V3 compatibility operation.
inline const CatalogEntry* findLegacyOperation(LegacyFamily family, uint32_t operation)
{
    for (const CatalogEntry& entry : kCatalog)
    {
        if (entry.legacyFamily == family && entry.legacyOperation == operation)
            return &entry;
    }
    return nullptr;
}

/// Returns the one established catalog row that exactly matches a complete typed operation.
inline const CatalogEntry* find(const SlangNVVMValueOperationDesc_4& desc)
{
    if (desc.structureSize != sizeof(desc) || (!desc.operandTypes && desc.operandCount))
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

} // namespace NVVMSemantics
} // namespace Slang
