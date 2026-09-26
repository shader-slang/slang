#pragma once

#include "slang-nvvm-ir-builder-api.h"

namespace Slang
{
namespace NVVMSemantics
{

/// Identifies a parameterized operation family outside the fixed exact-operation catalog.
enum class ValueOperationFamily : uint32_t
{
    None,
    IntegerUnary,
    IntegerBit,
    IntegerBinary,
    IntegerCompare,
    FloatUnary,
    FloatBinary,
    FloatTernary,
    FloatClassification,
    FloatSign,
    FloatCompare,
    BooleanUnary,
    BooleanBinary,
    BooleanCompare,
    IntegerConvert,
    IntegerToFloat,
    FloatToInteger,
    FloatConvert,
    BFloat16Convert,
    Float8Widen,
    BFloat16Dot,
    BitReinterpret,
    Select,
};

struct ValueOperationFamilyResolution
{
    ValueOperationFamily family = ValueOperationFamily::None;
    const char* diagnosticName = nullptr;
    bool requiresCUDADeviceLibrary = false;
};

/// Describes one established semantic overload from canonical Slang values to the provider ABI.
struct CatalogEntry
{
    SlangNVVMValueOperation operation;
    SlangNVVMValueTypeDesc resultType;
    SlangNVVMValueTypeDesc operandTypes[3];
    uint32_t operandCount;
    const char* diagnosticName;
    bool requiresCUDADeviceLibrary = false;
};

inline constexpr SlangNVVMValueTypeDesc kNoType = {};
inline constexpr SlangNVVMValueTypeDesc kVoid = {
    SLANG_NVVM_VALUE_TYPE_VOID,
    0,
    0,
};
inline constexpr SlangNVVMValueTypeDesc kBool = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, 1};
inline constexpr SlangNVVMValueTypeDesc kSignedI8 = {
    SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
    8,
    1,
};
inline constexpr SlangNVVMValueTypeDesc kUnsignedI8 = {
    SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    8,
    1,
};
inline constexpr SlangNVVMValueTypeDesc kSignedI64 = {
    SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
    64,
    1,
};
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
inline constexpr SlangNVVMValueTypeDesc kSignedI16 = {
    SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
    16,
    1,
};
inline constexpr SlangNVVMValueTypeDesc kUnsignedI16 = {
    SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    16,
    1,
};
inline constexpr SlangNVVMValueTypeDesc kUnsignedI64 = {
    SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
    64,
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
inline constexpr SlangNVVMValueTypeDesc kFloat16 = {
    SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
    16,
    1,
};
inline constexpr SlangNVVMValueTypeDesc kBFloat16 = {SLANG_NVVM_VALUE_TYPE_BFLOAT16, 16, 1};
inline constexpr SlangNVVMValueTypeDesc kFloatE4M3 = {SLANG_NVVM_VALUE_TYPE_FLOAT_E4M3, 8, 1};
inline constexpr SlangNVVMValueTypeDesc kFloatE5M2 = {SLANG_NVVM_VALUE_TYPE_FLOAT_E5M2, 8, 1};
inline constexpr SlangNVVMValueTypeDesc kFloat64 = {
    SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
    64,
    1,
};
inline constexpr SlangNVVMValueTypeDesc kSignedI32x2 = {
    SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
    32,
    2,
};

// This is the only table that maps an established typed semantic to its provider operation.
// Producer and target spellings are deliberately absent from the typed provider contract.
inline constexpr CatalogEntry kCatalog[] = {
    {
        SLANG_NVVM_VALUE_OP_ADD,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 addition",
    },
    {
        SLANG_NVVM_VALUE_OP_SUBTRACT,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 subtraction",
    },
    {
        SLANG_NVVM_VALUE_OP_MULTIPLY,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 multiplication",
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_AND,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 bitwise AND",
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_OR,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 bitwise OR",
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_XOR,
        kSignedI32,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 bitwise XOR",
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_NOT,
        kSignedI32,
        {kSignedI32, kNoType, kNoType},
        1,
        "signed i32 bitwise NOT",
    },
    {
        SLANG_NVVM_VALUE_OP_NEGATE,
        kSignedI32,
        {kSignedI32, kNoType, kNoType},
        1,
        "signed i32 arithmetic negation",
    },
    {
        SLANG_NVVM_VALUE_OP_EQUAL,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 equality comparison",
    },
    {
        SLANG_NVVM_VALUE_OP_NOT_EQUAL,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 inequality comparison",
    },
    {
        SLANG_NVVM_VALUE_OP_LESS_THAN,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 less-than comparison",
    },
    {
        SLANG_NVVM_VALUE_OP_GREATER_THAN,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 greater-than comparison",
    },
    {
        SLANG_NVVM_VALUE_OP_LESS_EQUAL,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 less-than-or-equal comparison",
    },
    {
        SLANG_NVVM_VALUE_OP_GREATER_EQUAL,
        kBool,
        {kSignedI32, kSignedI32, kNoType},
        2,
        "signed i32 greater-than-or-equal comparison",
    },
    {
        SLANG_NVVM_VALUE_OP_ADD,
        kFloat32,
        {kFloat32, kFloat32, kNoType},
        2,
        "float32 addition",
    },
    {
        SLANG_NVVM_VALUE_OP_SUBTRACT,
        kFloat32,
        {kFloat32, kFloat32, kNoType},
        2,
        "float32 subtraction",
    },
    {
        SLANG_NVVM_VALUE_OP_MULTIPLY,
        kFloat32,
        {kFloat32, kFloat32, kNoType},
        2,
        "float32 multiplication",
    },
    {
        SLANG_NVVM_VALUE_OP_DIVIDE,
        kFloat32,
        {kFloat32, kFloat32, kNoType},
        2,
        "float32 division",
    },
    {
        SLANG_NVVM_VALUE_OP_NEGATE,
        kFloat32,
        {kFloat32, kNoType, kNoType},
        1,
        "float32 negation",
    },
    {
        SLANG_NVVM_VALUE_OP_EQUAL,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        "float32 ordered equality",
    },
    {
        SLANG_NVVM_VALUE_OP_NOT_EQUAL,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        "float32 unordered inequality",
    },
    {
        SLANG_NVVM_VALUE_OP_LESS_THAN,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        "float32 ordered less-than",
    },
    {
        SLANG_NVVM_VALUE_OP_GREATER_THAN,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        "float32 ordered greater-than",
    },
    {
        SLANG_NVVM_VALUE_OP_LESS_EQUAL,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        "float32 ordered less-than-or-equal",
    },
    {
        SLANG_NVVM_VALUE_OP_GREATER_EQUAL,
        kBool,
        {kFloat32, kFloat32, kNoType},
        2,
        "float32 ordered greater-than-or-equal",
    },
    {
        SLANG_NVVM_VALUE_OP_FLOAT_CONVERT,
        kFloat16,
        {kFloat32, kNoType, kNoType},
        1,
        "floating-point width conversion",
    },
    {
        SLANG_NVVM_VALUE_OP_FLOAT_CONVERT,
        kFloat32,
        {kFloat16, kNoType, kNoType},
        1,
        "floating-point width conversion",
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        kSignedI32,
        {kFloat32, kNoType, kNoType},
        1,
        "32-bit value reinterpretation",
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        kSignedI32,
        {kUnsignedI32, kNoType, kNoType},
        1,
        "32-bit value reinterpretation",
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        kUnsignedI32,
        {kFloat32, kNoType, kNoType},
        1,
        "32-bit value reinterpretation",
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        kUnsignedI32,
        {kSignedI32, kNoType, kNoType},
        1,
        "32-bit value reinterpretation",
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        kFloat32,
        {kSignedI32, kNoType, kNoType},
        1,
        "32-bit value reinterpretation",
    },
    {
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        kFloat32,
        {kUnsignedI32, kNoType, kNoType},
        1,
        "32-bit value reinterpretation",
    },
    {
        SLANG_NVVM_VALUE_OP_SQRT,
        kFloat32,
        {kFloat32, kNoType, kNoType},
        1,
        "float32 square root",
    },
    {
        SLANG_NVVM_VALUE_OP_TRUNC,
        kFloat32,
        {kFloat32, kNoType, kNoType},
        1,
        "float32 truncation",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_SIN,
        kFloat32,
        {kFloat32, kNoType, kNoType},
        1,
        "float32 sine",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_COS,
        kFloat32,
        {kFloat32, kNoType, kNoType},
        1,
        "float32 cosine",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_SIN,
        kFloat64,
        {kFloat64, kNoType, kNoType},
        1,
        "float64 sine",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_COS,
        kFloat64,
        {kFloat64, kNoType, kNoType},
        1,
        "float64 cosine",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_FREXP_FRACTION,
        kFloat32,
        {kFloat32, kNoType, kNoType},
        1,
        "float32 frexp fraction",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_FREXP_EXPONENT,
        kSignedI32,
        {kFloat32, kNoType, kNoType},
        1,
        "float32 frexp exponent",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_FREXP_FRACTION,
        kFloat64,
        {kFloat64, kNoType, kNoType},
        1,
        "float64 frexp fraction",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_FREXP_EXPONENT,
        kSignedI32,
        {kFloat64, kNoType, kNoType},
        1,
        "float64 frexp exponent",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_MODF_FRACTION,
        kFloat32,
        {kFloat32, kNoType, kNoType},
        1,
        "float32 modf fraction",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_MODF_INTEGRAL,
        kFloat32,
        {kFloat32, kNoType, kNoType},
        1,
        "float32 modf integral part",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_MODF_FRACTION,
        kFloat64,
        {kFloat64, kNoType, kNoType},
        1,
        "float64 modf fraction",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_MODF_INTEGRAL,
        kFloat64,
        {kFloat64, kNoType, kNoType},
        1,
        "float64 modf integral part",
        true,
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX,
        kUnsignedI32,
        {kNoType, kNoType, kNoType},
        0,
        "wave lane index intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_LANE_COUNT,
        kUnsignedI32,
        {kNoType, kNoType, kNoType},
        0,
        "wave lane count intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kUnsignedI32,
        {kUnsignedI32, kUnsignedI32, kSignedI32},
        3,
        "UInt wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kSignedI32,
        {kUnsignedI32, kSignedI32, kSignedI32},
        3,
        "Int wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kFloat32,
        {kUnsignedI32, kFloat32, kSignedI32},
        3,
        "Float wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kBool,
        {kUnsignedI32, kBool, kSignedI32},
        3,
        "Bool wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kSignedI8,
        {kUnsignedI32, kSignedI8, kSignedI32},
        3,
        "SignedI8 wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kUnsignedI8,
        {kUnsignedI32, kUnsignedI8, kSignedI32},
        3,
        "UnsignedI8 wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kSignedI16,
        {kUnsignedI32, kSignedI16, kSignedI32},
        3,
        "SignedI16 wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kUnsignedI16,
        {kUnsignedI32, kUnsignedI16, kSignedI32},
        3,
        "UnsignedI16 wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kSignedI64,
        {kUnsignedI32, kSignedI64, kSignedI32},
        3,
        "SignedI64 wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kUnsignedI64,
        {kUnsignedI32, kUnsignedI64, kSignedI32},
        3,
        "UnsignedI64 wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kFloat16,
        {kUnsignedI32, kFloat16, kSignedI32},
        3,
        "Float16 wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        kFloat64,
        {kUnsignedI32, kFloat64, kSignedI32},
        3,
        "Float64 wave read-lane-at intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_CLOCK,
        kUnsignedI32,
        {kNoType, kNoType, kNoType},
        0,
        "CUDA clock counter read",
    },
    {
        SLANG_NVVM_VALUE_OP_CLOCK64,
        kSignedI64,
        {kNoType, kNoType, kNoType},
        0,
        "CUDA clock64 counter read",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_ACTIVE_MASK,
        kUnsignedI32,
        {kNoType, kNoType, kNoType},
        0,
        "hardware wave active mask",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT,
        kUnsignedI32,
        {kUnsignedI32, kBool, kNoType},
        2,
        "wave-mask ballot intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        kUnsignedI32,
        {kUnsignedI32, kUnsignedI32, kNoType},
        2,
        "UInt wave read-lane-first intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        kSignedI32,
        {kUnsignedI32, kSignedI32, kNoType},
        2,
        "Int wave read-lane-first intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST,
        kFloat32,
        {kUnsignedI32, kFloat32, kNoType},
        2,
        "Float wave read-lane-first intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_IS_FIRST_LANE,
        kBool,
        {kUnsignedI32, kNoType, kNoType},
        1,
        "wave-mask is-first-lane intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ANY_TRUE,
        kBool,
        {kUnsignedI32, kBool, kNoType},
        2,
        "wave-mask any-true intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_TRUE,
        kBool,
        {kUnsignedI32, kBool, kNoType},
        2,
        "wave-mask all-true intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        kBool,
        {kUnsignedI32, kSignedI32, kNoType},
        2,
        "signed-i32 wave-mask all-equal intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        kBool,
        {kUnsignedI32, kUnsignedI32, kNoType},
        2,
        "unsigned-i32 wave-mask all-equal intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL,
        kBool,
        {kUnsignedI32, kFloat32, kNoType},
        2,
        "float32 wave-mask all-equal intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH,
        kUnsignedI32,
        {kUnsignedI32, kSignedI32, kNoType},
        2,
        "signed-i32 wave-mask match intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH,
        kUnsignedI32,
        {kUnsignedI32, kUnsignedI32, kNoType},
        2,
        "unsigned-i32 wave-mask match intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH,
        kUnsignedI32,
        {kUnsignedI32, kFloat32, kNoType},
        2,
        "float32 wave-mask match intrinsic",
    },
    {
        SLANG_NVVM_VALUE_OP_THREAD_INDEX,
        kUnsignedI32x3,
        {kNoType, kNoType, kNoType},
        0,
        "CUDA thread index",
    },
    {
        SLANG_NVVM_VALUE_OP_BLOCK_INDEX,
        kUnsignedI32x3,
        {kNoType, kNoType, kNoType},
        0,
        "CUDA block index",
    },
    {
        SLANG_NVVM_VALUE_OP_BLOCK_DIMENSIONS,
        kUnsignedI32x3,
        {kNoType, kNoType, kNoType},
        0,
        "CUDA block dimensions",
    },
    {
        SLANG_NVVM_VALUE_OP_GRID_DIMENSIONS,
        kUnsignedI32x3,
        {kNoType, kNoType, kNoType},
        0,
        "CUDA grid dimensions",
    },
    {
        SLANG_NVVM_VALUE_OP_WORKGROUP_BARRIER,
        kVoid,
        {kNoType, kNoType, kNoType},
        0,
        "CUDA workgroup barrier",
    },
    {
        SLANG_NVVM_VALUE_OP_DEVICE_MEMORY_BARRIER,
        kVoid,
        {kNoType, kNoType, kNoType},
        0,
        "CUDA device memory barrier",
    },
    {
        SLANG_NVVM_VALUE_OP_WORKGROUP_MEMORY_BARRIER,
        kVoid,
        {kNoType, kNoType, kNoType},
        0,
        "CUDA workgroup memory fence",
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

inline bool isSelectedIntegerValue(const SlangNVVMValueTypeDesc& type)
{
    const bool isInteger = type.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                           type.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER;
    const bool isSelectedWidth =
        type.bitWidth == 8 || type.bitWidth == 16 || type.bitWidth == 32 || type.bitWidth == 64;
    return isInteger && isSelectedWidth && type.laneCount >= 1 && type.laneCount <= 4;
}

inline bool isSelectedFloatValue(const SlangNVVMValueTypeDesc& type)
{
    const bool isSelectedWidth = type.bitWidth == 16 || type.bitWidth == 32 || type.bitWidth == 64;
    return type.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT && isSelectedWidth &&
           type.laneCount >= 1 && type.laneCount <= 4;
}

inline bool isSelectedBoolValue(const SlangNVVMValueTypeDesc& type)
{
    return type.kind == SLANG_NVVM_VALUE_TYPE_BOOL && type.bitWidth == 1 && type.laneCount >= 1 &&
           type.laneCount <= 4;
}

/// Returns whether both values have the same semantic element type, ignoring lane count.
inline bool haveSameElementType(
    const SlangNVVMValueTypeDesc& left,
    const SlangNVVMValueTypeDesc& right)
{
    return left.kind == right.kind && left.bitWidth == right.bitWidth;
}

/// Returns whether an operand has either the result width or the scalar width used for broadcast.
inline bool hasComponentWiseLanes(
    const SlangNVVMValueTypeDesc& resultType,
    const SlangNVVMValueTypeDesc& operandType)
{
    return operandType.laneCount == 1 || operandType.laneCount == resultType.laneCount;
}

/// Checks a canonical component-wise binary shape, including one scalar-broadcast operand.
inline bool isComponentWiseBinary(
    const SlangNVVMValueTypeDesc& resultType,
    const SlangNVVMValueTypeDesc& leftType,
    const SlangNVVMValueTypeDesc& rightType)
{
    if (!haveSameElementType(resultType, leftType) || !haveSameElementType(resultType, rightType))
    {
        return false;
    }
    const bool hasResultWidth = resultType.laneCount == 1 ||
                                leftType.laneCount == resultType.laneCount ||
                                rightType.laneCount == resultType.laneCount;
    return hasComponentWiseLanes(resultType, leftType) &&
           hasComponentWiseLanes(resultType, rightType) && hasResultWidth;
}

/// Resolves the bounded, dimensioned numeric families added after the frozen exact catalog.
inline bool resolveValueOperationFamily(
    const SlangNVVMValueOperationDesc& desc,
    ValueOperationFamilyResolution& outResolution)
{
    outResolution = {};
    if (!desc.operandTypes && desc.operandCount)
        return false;

    const bool isUnaryIntegerValue = desc.operandCount == 1 &&
                                     isSelectedIntegerValue(desc.resultType) &&
                                     areSameType(desc.resultType, desc.operandTypes[0]);
    const bool isUnaryInteger = isUnaryIntegerValue && isSelectedScalarInteger(desc.resultType);
    const bool isSignedUnaryInteger =
        isUnaryInteger && desc.resultType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER;
    if ((isUnaryIntegerValue && desc.operation == SLANG_NVVM_VALUE_OP_BIT_NOT) ||
        (isUnaryInteger && desc.operation == SLANG_NVVM_VALUE_OP_NEGATE) ||
        (isSignedUnaryInteger && desc.operation == SLANG_NVVM_VALUE_OP_ABS))
    {
        outResolution = {
            ValueOperationFamily::IntegerUnary,
            "parameterized integer unary operation"};
        return true;
    }

    const bool isIntegerBitOperand =
        desc.operandCount == 1 && isSelectedScalarInteger(desc.operandTypes[0]);
    const bool isSameTypeBitResult =
        isIntegerBitOperand && areSameType(desc.resultType, desc.operandTypes[0]);
    const bool isUnsignedI32BitResult =
        isIntegerBitOperand && areSameType(desc.resultType, kUnsignedI32);
    if ((isSameTypeBitResult && desc.operation == SLANG_NVVM_VALUE_OP_REVERSE_BITS) ||
        (isUnsignedI32BitResult && (desc.operation == SLANG_NVVM_VALUE_OP_COUNT_BITS ||
                                    desc.operation == SLANG_NVVM_VALUE_OP_FIRST_BIT_HIGH ||
                                    desc.operation == SLANG_NVVM_VALUE_OP_FIRST_BIT_LOW)))
    {
        outResolution = {ValueOperationFamily::IntegerBit, "scalar integer bit operation"};
        return true;
    }

    const bool isOrdinaryBinaryInteger =
        desc.operandCount == 2 && isSelectedIntegerValue(desc.resultType) &&
        isSelectedIntegerValue(desc.operandTypes[0]) &&
        isSelectedIntegerValue(desc.operandTypes[1]) &&
        isComponentWiseBinary(desc.resultType, desc.operandTypes[0], desc.operandTypes[1]);
    const bool isIntegerShift = desc.operandCount == 2 && isSelectedIntegerValue(desc.resultType) &&
                                isSelectedIntegerValue(desc.operandTypes[0]) &&
                                isSelectedIntegerValue(desc.operandTypes[1]) &&
                                areSameType(desc.resultType, desc.operandTypes[0]) &&
                                hasComponentWiseLanes(desc.resultType, desc.operandTypes[1]);
    const bool isScalarIntegerMinMax = desc.operandCount == 2 &&
                                       isSelectedScalarInteger(desc.resultType) &&
                                       areSameType(desc.resultType, desc.operandTypes[0]) &&
                                       areSameType(desc.resultType, desc.operandTypes[1]);
    if ((isOrdinaryBinaryInteger && (desc.operation == SLANG_NVVM_VALUE_OP_ADD ||
                                     desc.operation == SLANG_NVVM_VALUE_OP_SUBTRACT ||
                                     desc.operation == SLANG_NVVM_VALUE_OP_MULTIPLY ||
                                     desc.operation == SLANG_NVVM_VALUE_OP_DIVIDE ||
                                     desc.operation == SLANG_NVVM_VALUE_OP_BIT_AND ||
                                     desc.operation == SLANG_NVVM_VALUE_OP_BIT_OR ||
                                     desc.operation == SLANG_NVVM_VALUE_OP_BIT_XOR ||
                                     desc.operation == SLANG_NVVM_VALUE_OP_REMAINDER)) ||
        (isScalarIntegerMinMax && (desc.operation == SLANG_NVVM_VALUE_OP_MIN ||
                                   desc.operation == SLANG_NVVM_VALUE_OP_MAX)) ||
        (isIntegerShift && (desc.operation == SLANG_NVVM_VALUE_OP_SHIFT_LEFT ||
                            desc.operation == SLANG_NVVM_VALUE_OP_SHIFT_RIGHT)))
    {
        outResolution = {
            ValueOperationFamily::IntegerBinary,
            "parameterized integer binary operation"};
        return true;
    }

    SlangNVVMValueTypeDesc integerCompareResultElement = {};
    if (desc.operandCount == 2)
    {
        integerCompareResultElement = desc.operandTypes[0];
        integerCompareResultElement.laneCount = desc.resultType.laneCount;
    }
    const bool isIntegerCompare = desc.operandCount == 2 && isSelectedBoolValue(desc.resultType) &&
                                  isSelectedIntegerValue(desc.operandTypes[0]) &&
                                  isSelectedIntegerValue(desc.operandTypes[1]) &&
                                  isComponentWiseBinary(
                                      integerCompareResultElement,
                                      desc.operandTypes[0],
                                      desc.operandTypes[1]);
    if (isIntegerCompare && desc.operation >= SLANG_NVVM_VALUE_OP_EQUAL &&
        desc.operation <= SLANG_NVVM_VALUE_OP_GREATER_EQUAL)
    {
        outResolution = {ValueOperationFamily::IntegerCompare, "parameterized integer comparison"};
        return true;
    }

    SlangNVVMValueTypeDesc floatCompareResultElement = {};
    if (desc.operandCount == 2)
    {
        floatCompareResultElement = desc.operandTypes[0];
        floatCompareResultElement.laneCount = desc.resultType.laneCount;
    }
    const bool isFloatCompare = desc.operandCount == 2 && isSelectedBoolValue(desc.resultType) &&
                                isSelectedFloatValue(desc.operandTypes[0]) &&
                                isSelectedFloatValue(desc.operandTypes[1]) &&
                                isComponentWiseBinary(
                                    floatCompareResultElement,
                                    desc.operandTypes[0],
                                    desc.operandTypes[1]);
    if (isFloatCompare && desc.operation >= SLANG_NVVM_VALUE_OP_EQUAL &&
        desc.operation <= SLANG_NVVM_VALUE_OP_GREATER_EQUAL)
    {
        outResolution = {
            ValueOperationFamily::FloatCompare,
            "parameterized floating-point comparison"};
        return true;
    }

    const bool isUnaryFloat = desc.operandCount == 1 && isSelectedFloatValue(desc.resultType) &&
                              areSameType(desc.resultType, desc.operandTypes[0]);
    if (isUnaryFloat && desc.operation == SLANG_NVVM_VALUE_OP_NEGATE)
    {
        outResolution = {
            ValueOperationFamily::FloatUnary,
            "parameterized floating-point unary operation"};
        return true;
    }

    const bool isScalarFloat16Abs = isUnaryFloat && desc.resultType.bitWidth == 16 &&
                                    desc.resultType.laneCount == 1 &&
                                    desc.operation == SLANG_NVVM_VALUE_OP_ABS;
    const bool isScalarFloat32Or64Unary =
        isUnaryFloat && (desc.resultType.bitWidth == 32 || desc.resultType.bitWidth == 64) &&
        desc.resultType.laneCount == 1;
    const bool isScalarMathUnary =
        desc.operation == SLANG_NVVM_VALUE_OP_ABS || desc.operation == SLANG_NVVM_VALUE_OP_ACOS ||
        desc.operation == SLANG_NVVM_VALUE_OP_ASIN || desc.operation == SLANG_NVVM_VALUE_OP_ATAN ||
        desc.operation == SLANG_NVVM_VALUE_OP_CEIL || desc.operation == SLANG_NVVM_VALUE_OP_EXP ||
        desc.operation == SLANG_NVVM_VALUE_OP_EXP2 || desc.operation == SLANG_NVVM_VALUE_OP_FLOOR ||
        desc.operation == SLANG_NVVM_VALUE_OP_FRAC || desc.operation == SLANG_NVVM_VALUE_OP_LOG ||
        desc.operation == SLANG_NVVM_VALUE_OP_LOG2 || desc.operation == SLANG_NVVM_VALUE_OP_LOG10 ||
        desc.operation == SLANG_NVVM_VALUE_OP_ROUND ||
        desc.operation == SLANG_NVVM_VALUE_OP_RSQRT || desc.operation == SLANG_NVVM_VALUE_OP_SQRT ||
        desc.operation == SLANG_NVVM_VALUE_OP_SINH || desc.operation == SLANG_NVVM_VALUE_OP_COSH ||
        desc.operation == SLANG_NVVM_VALUE_OP_TANH || desc.operation == SLANG_NVVM_VALUE_OP_TAN ||
        desc.operation == SLANG_NVVM_VALUE_OP_TRUNC;
    if (isScalarFloat16Abs || (isScalarFloat32Or64Unary && isScalarMathUnary))
    {
        outResolution = {
            ValueOperationFamily::FloatUnary,
            "scalar floating-point math operation",
            desc.resultType.bitWidth != 16 && desc.operation != SLANG_NVVM_VALUE_OP_SQRT,
        };
        return true;
    }

    const bool isScalarFloat32Or64Operand =
        desc.operandCount == 1 &&
        desc.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
        (desc.operandTypes[0].bitWidth == 32 || desc.operandTypes[0].bitWidth == 64) &&
        desc.operandTypes[0].laneCount == 1;
    if (isScalarFloat32Or64Operand && desc.operation == SLANG_NVVM_VALUE_OP_IS_NAN &&
        areSameType(desc.resultType, kBool))
    {
        outResolution = {
            ValueOperationFamily::FloatClassification,
            "scalar floating-point classification"};
        return true;
    }
    if (isScalarFloat32Or64Operand && desc.operation == SLANG_NVVM_VALUE_OP_SIGN &&
        areSameType(desc.resultType, kSignedI32))
    {
        outResolution = {ValueOperationFamily::FloatSign, "scalar floating-point sign"};
        return true;
    }

    const bool isBinaryFloat =
        desc.operandCount == 2 && isSelectedFloatValue(desc.resultType) &&
        isSelectedFloatValue(desc.operandTypes[0]) && isSelectedFloatValue(desc.operandTypes[1]) &&
        isComponentWiseBinary(desc.resultType, desc.operandTypes[0], desc.operandTypes[1]);
    if (isBinaryFloat && (desc.operation == SLANG_NVVM_VALUE_OP_ADD ||
                          desc.operation == SLANG_NVVM_VALUE_OP_SUBTRACT ||
                          desc.operation == SLANG_NVVM_VALUE_OP_MULTIPLY ||
                          desc.operation == SLANG_NVVM_VALUE_OP_DIVIDE ||
                          desc.operation == SLANG_NVVM_VALUE_OP_REMAINDER))
    {
        outResolution = {
            ValueOperationFamily::FloatBinary,
            "parameterized floating-point binary operation"};
        return true;
    }

    const bool isScalarFloat32Or64Binary =
        desc.operandCount == 2 && desc.resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
        (desc.resultType.bitWidth == 32 || desc.resultType.bitWidth == 64) &&
        desc.resultType.laneCount == 1 && areSameType(desc.resultType, desc.operandTypes[0]) &&
        areSameType(desc.resultType, desc.operandTypes[1]);
    if (isScalarFloat32Or64Binary &&
        (desc.operation == SLANG_NVVM_VALUE_OP_ATAN2 ||
         desc.operation == SLANG_NVVM_VALUE_OP_FMOD || desc.operation == SLANG_NVVM_VALUE_OP_POW))
    {
        outResolution = {
            ValueOperationFamily::FloatBinary,
            "scalar floating-point math operation",
            true,
        };
        return true;
    }

    const bool isLibdeviceBinaryFloat =
        desc.operandCount == 2 && desc.resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
        (desc.resultType.bitWidth == 32 || desc.resultType.bitWidth == 64) &&
        desc.resultType.laneCount == 1 && areSameType(desc.resultType, desc.operandTypes[0]) &&
        areSameType(desc.resultType, desc.operandTypes[1]);
    if (isLibdeviceBinaryFloat &&
        (desc.operation == SLANG_NVVM_VALUE_OP_MIN || desc.operation == SLANG_NVVM_VALUE_OP_MAX))
    {
        outResolution = {
            ValueOperationFamily::FloatBinary,
            "scalar floating-point minimum or maximum",
            true,
        };
        return true;
    }

    const bool isScalarFloat32Or64Ternary =
        desc.operandCount == 3 && desc.resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
        (desc.resultType.bitWidth == 32 || desc.resultType.bitWidth == 64) &&
        desc.resultType.laneCount == 1 && areSameType(desc.resultType, desc.operandTypes[0]) &&
        areSameType(desc.resultType, desc.operandTypes[1]) &&
        areSameType(desc.resultType, desc.operandTypes[2]);
    if (isScalarFloat32Or64Ternary && desc.operation == SLANG_NVVM_VALUE_OP_FMA)
    {
        outResolution = {ValueOperationFamily::FloatTernary, "scalar fused multiply-add", true};
        return true;
    }

    const bool isUnaryBoolean = desc.operandCount == 1 && isSelectedBoolValue(desc.resultType) &&
                                areSameType(desc.resultType, desc.operandTypes[0]);
    if (isUnaryBoolean && desc.operation == SLANG_NVVM_VALUE_OP_BIT_NOT)
    {
        outResolution = {
            ValueOperationFamily::BooleanUnary,
            "parameterized Boolean unary operation"};
        return true;
    }

    const bool isBinaryBoolean =
        desc.operandCount == 2 && isSelectedBoolValue(desc.resultType) &&
        isSelectedBoolValue(desc.operandTypes[0]) && isSelectedBoolValue(desc.operandTypes[1]) &&
        isComponentWiseBinary(desc.resultType, desc.operandTypes[0], desc.operandTypes[1]);
    if (isBinaryBoolean && (desc.operation == SLANG_NVVM_VALUE_OP_BIT_AND ||
                            desc.operation == SLANG_NVVM_VALUE_OP_BIT_OR))
    {
        outResolution = {
            ValueOperationFamily::BooleanBinary,
            "parameterized Boolean binary operation"};
        return true;
    }
    if (isBinaryBoolean && (desc.operation == SLANG_NVVM_VALUE_OP_EQUAL ||
                            desc.operation == SLANG_NVVM_VALUE_OP_NOT_EQUAL))
    {
        outResolution = {ValueOperationFamily::BooleanCompare, "parameterized Boolean comparison"};
        return true;
    }

    if (desc.operation == SLANG_NVVM_VALUE_OP_INTEGER_CONVERT && desc.operandCount == 1 &&
        isSelectedIntegerValue(desc.resultType) &&
        (isSelectedIntegerValue(desc.operandTypes[0]) ||
         isSelectedBoolValue(desc.operandTypes[0])) &&
        desc.resultType.laneCount == desc.operandTypes[0].laneCount &&
        !areSameType(desc.resultType, desc.operandTypes[0]))
    {
        outResolution = {ValueOperationFamily::IntegerConvert, "explicit integer conversion"};
        return true;
    }
    if (desc.operation == SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT && desc.operandCount == 1 &&
        isSelectedFloatValue(desc.resultType) &&
        (isSelectedIntegerValue(desc.operandTypes[0]) ||
         isSelectedBoolValue(desc.operandTypes[0])) &&
        desc.resultType.laneCount == desc.operandTypes[0].laneCount)
    {
        outResolution = {
            ValueOperationFamily::IntegerToFloat,
            "integer-to-floating-point conversion"};
        return true;
    }
    if (desc.operation == SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER && desc.operandCount == 1 &&
        isSelectedIntegerValue(desc.resultType) && isSelectedFloatValue(desc.operandTypes[0]) &&
        desc.resultType.laneCount == desc.operandTypes[0].laneCount)
    {
        outResolution = {
            ValueOperationFamily::FloatToInteger,
            "floating-point-to-integer conversion"};
        return true;
    }
    if (desc.operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT && desc.operandCount == 1 &&
        isSelectedFloatValue(desc.resultType) && isSelectedFloatValue(desc.operandTypes[0]) &&
        desc.resultType.laneCount == desc.operandTypes[0].laneCount &&
        desc.resultType.bitWidth != desc.operandTypes[0].bitWidth)
    {
        outResolution = {ValueOperationFamily::FloatConvert, "floating-point width conversion"};
        return true;
    }

    // The CUDA prelude owns dot overloads for BF16 widths 2, 3, and 4. Preserve the
    // specialized signature rather than admitting BF16 through ordinary floating arithmetic.
    if (desc.operation == SLANG_NVVM_VALUE_OP_BFLOAT16_DOT && desc.operandCount == 2 &&
        areSameType(desc.resultType, kBFloat16) &&
        haveSameElementType(desc.operandTypes[0], kBFloat16) &&
        desc.operandTypes[0].laneCount >= 2 && desc.operandTypes[0].laneCount <= 4 &&
        areSameType(desc.operandTypes[0], desc.operandTypes[1]))
    {
        outResolution = {ValueOperationFamily::BFloat16Dot, "source-ordered BF16 dot"};
        return true;
    }

    // BF16 shares neither Half arithmetic nor generic integer/float conversion recipes.
    // Consider BFloat16(asfloat(bits)): the canonical FloatCast narrows exactly once from
    // Float32. Integer construction cannot use this path because an intermediate Float32
    // rounding can change the BF16 result (for example, integer 16842753).
    if (desc.operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT && desc.operandCount == 1 &&
        desc.resultType.laneCount >= 1 && desc.resultType.laneCount <= 4 &&
        desc.resultType.laneCount == desc.operandTypes[0].laneCount &&
        ((desc.resultType.kind == SLANG_NVVM_VALUE_TYPE_BFLOAT16 &&
          desc.resultType.bitWidth == 16 &&
          desc.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
          desc.operandTypes[0].bitWidth == 32) ||
         (desc.resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
          desc.resultType.bitWidth == 32 &&
          desc.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_BFLOAT16 &&
          desc.operandTypes[0].bitWidth == 16)))
    {
        outResolution = {
            ValueOperationFamily::BFloat16Convert,
            "component BF16/Float32 conversion"};
        return true;
    }
    if (desc.operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET && desc.operandCount == 1 &&
        ((areSameType(desc.resultType, kBFloat16) &&
          (areSameType(desc.operandTypes[0], kUnsignedI16) ||
           areSameType(desc.operandTypes[0], kSignedI16))) ||
         (areSameType(desc.operandTypes[0], kBFloat16) &&
          (areSameType(desc.resultType, kUnsignedI16) ||
           areSameType(desc.resultType, kSignedI16)))))
    {
        outResolution = {ValueOperationFamily::BitReinterpret, "scalar BF16 bit transport"};
        return true;
    }

    const bool isFloat8Result =
        areSameType(desc.resultType, kFloatE4M3) || areSameType(desc.resultType, kFloatE5M2);
    const bool isFloat8Operand =
        desc.operandCount == 1 && (areSameType(desc.operandTypes[0], kFloatE4M3) ||
                                   areSameType(desc.operandTypes[0], kFloatE5M2));
    // A canonical FP8 scalar widens exactly to Float32. For example, E4M3 byte 1
    // represents 2^-9, not the integer 1 or an IEEE eight-bit float. Keep reverse
    // narrowing separate because its CUDA saturation policy is a different contract.
    if (desc.operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT && isFloat8Operand &&
        areSameType(desc.resultType, kFloat32))
    {
        outResolution = {ValueOperationFamily::Float8Widen, "scalar FP8-to-Float32 widening"};
        return true;
    }

    const bool hasBitResult = isFloat8Result || isSelectedIntegerValue(desc.resultType) ||
                              isSelectedFloatValue(desc.resultType);
    const bool hasBitOperand = desc.operandCount == 1 &&
                               (isFloat8Operand || isSelectedIntegerValue(desc.operandTypes[0]) ||
                                isSelectedFloatValue(desc.operandTypes[0]));
    const bool hasDistinctEqualWidthBitTypes =
        hasBitResult && hasBitOperand && !areSameType(desc.resultType, desc.operandTypes[0]) &&
        desc.resultType.bitWidth * desc.resultType.laneCount ==
            desc.operandTypes[0].bitWidth * desc.operandTypes[0].laneCount;
    if (desc.operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET && hasBitResult && hasBitOperand &&
        hasDistinctEqualWidthBitTypes)
    {
        outResolution = {ValueOperationFamily::BitReinterpret, "bitwise value reinterpretation"};
        return true;
    }

    const bool hasSelectedResult = isFloat8Result || areSameType(desc.resultType, kBFloat16) ||
                                   isSelectedBoolValue(desc.resultType) ||
                                   isSelectedIntegerValue(desc.resultType) ||
                                   isSelectedFloatValue(desc.resultType);
    if (desc.operation == SLANG_NVVM_VALUE_OP_SELECT && desc.operandCount == 3 &&
        hasSelectedResult && isSelectedBoolValue(desc.operandTypes[0]) &&
        desc.operandTypes[0].laneCount == desc.resultType.laneCount &&
        areSameType(desc.resultType, desc.operandTypes[1]) &&
        areSameType(desc.resultType, desc.operandTypes[2]))
    {
        outResolution = {ValueOperationFamily::Select, "typed value selection"};
        return true;
    }

    return false;
}

inline bool isSupported(const SlangNVVMValueOperationDesc& desc)
{
    ValueOperationFamilyResolution resolution;
    return find(desc) || resolveValueOperationFamily(desc, resolution);
}

/// Returns whether an atomic descriptor is in the currently established direct-NVVM family.
inline bool isSupported(const SlangNVVMAtomicOperationDesc& desc)
{
    const bool isAtomicAddressSpace = desc.addressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL ||
                                      desc.addressSpace == SLANG_NVVM_ADDRESS_SPACE_SHARED;
    const bool isSelectedInteger =
        (desc.valueType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
         desc.valueType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER) &&
        (desc.valueType.bitWidth == 32 || desc.valueType.bitWidth == 64) &&
        desc.valueType.laneCount == 1;
    const bool isF32 = desc.valueType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                       desc.valueType.bitWidth == 32 && desc.valueType.laneCount == 1;
    const bool isHalf2 = desc.valueType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                         desc.valueType.bitWidth == 16 && desc.valueType.laneCount == 2;
    const bool isSelectedIntegerReduction =
        isAtomicAddressSpace && isSelectedInteger &&
        (desc.operation == SLANG_NVVM_ATOMIC_OP_ADD ||
         desc.operation == SLANG_NVVM_ATOMIC_OP_BIT_AND ||
         desc.operation == SLANG_NVVM_ATOMIC_OP_BIT_OR ||
         desc.operation == SLANG_NVVM_ATOMIC_OP_BIT_XOR ||
         desc.operation == SLANG_NVVM_ATOMIC_OP_MIN || desc.operation == SLANG_NVVM_ATOMIC_OP_MAX);
    const bool isSelectedFloatingReduction =
        desc.addressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL &&
        desc.operation == SLANG_NVVM_ATOMIC_OP_ADD &&
        ((desc.valueType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
          (desc.valueType.bitWidth == 16 || desc.valueType.bitWidth == 32 ||
           desc.valueType.bitWidth == 64) &&
          desc.valueType.laneCount == 1) ||
         isHalf2);
    const bool isCommonMemoryOperation = isAtomicAddressSpace && (isSelectedInteger || isF32) &&
                                         (desc.operation == SLANG_NVVM_ATOMIC_OP_LOAD ||
                                          desc.operation == SLANG_NVVM_ATOMIC_OP_STORE ||
                                          desc.operation == SLANG_NVVM_ATOMIC_OP_EXCHANGE ||
                                          desc.operation == SLANG_NVVM_ATOMIC_OP_COMPARE_EXCHANGE);
    return (isSelectedIntegerReduction || isSelectedFloatingReduction || isCommonMemoryOperation) &&
           desc.memoryOrder == SLANG_NVVM_MEMORY_ORDER_RELAXED &&
           desc.failureMemoryOrder == SLANG_NVVM_MEMORY_ORDER_RELAXED;
}

} // namespace NVVMSemantics
} // namespace Slang
