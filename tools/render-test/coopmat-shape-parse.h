// coopmat-shape-parse.h
#pragma once

// Parsing for the -require-coopmat-shape render-test option: the grammar
// "<type>:<scope>:<m>:<n>:<k>" (for example "half:subgroup:8:8:16") parsed into a
// CooperativeMatrixDesc. The parsing lives in this header rather than options.cpp so the
// unit tests can pin the grammar without linking the whole render-test tool; options.cpp
// adds the diagnostic handling on top.

#include "core/slang-list.h"
#include "core/slang-string-util.h"
#include "core/slang-type-text-util.h"
#include "slang-rhi.h"
#include "slang.h"

namespace renderer_test
{
using namespace Slang;

// Returns the cooperative-matrix component type named by `typeName`, resolving the token
// the way a shader spells the type: through TypeTextUtil::findScalarType, so the canonical
// spellings ("half", "int8_t", "float_e4m3", ...) are the accepted names and a test author
// can copy the component type straight out of the shader's CoopMat declaration. Returns
// false when the token names no component type (bool, double and the 64-bit integer types
// have no cooperative-matrix component type), so the caller rejects it as a
// test-configuration error rather than silently gating the test on a default type.
inline bool tryGetCooperativeMatrixComponentTypeFromName(
    const UnownedStringSlice& typeName,
    rhi::CooperativeMatrixComponentType& outType)
{
    using rhi::CooperativeMatrixComponentType;
    switch (TypeTextUtil::findScalarType(typeName))
    {
    case slang::TypeReflection::ScalarType::Float16:
        outType = CooperativeMatrixComponentType::Float16;
        return true;
    case slang::TypeReflection::ScalarType::Float32:
        outType = CooperativeMatrixComponentType::Float32;
        return true;
    case slang::TypeReflection::ScalarType::BFloat16:
        outType = CooperativeMatrixComponentType::Bfloat16;
        return true;
    case slang::TypeReflection::ScalarType::FloatE4M3:
        outType = CooperativeMatrixComponentType::FloatE4M3;
        return true;
    case slang::TypeReflection::ScalarType::FloatE5M2:
        outType = CooperativeMatrixComponentType::FloatE5M2;
        return true;
    case slang::TypeReflection::ScalarType::Int8:
        outType = CooperativeMatrixComponentType::Int8;
        return true;
    case slang::TypeReflection::ScalarType::UInt8:
        outType = CooperativeMatrixComponentType::Uint8;
        return true;
    case slang::TypeReflection::ScalarType::Int16:
        outType = CooperativeMatrixComponentType::Int16;
        return true;
    case slang::TypeReflection::ScalarType::UInt16:
        outType = CooperativeMatrixComponentType::Uint16;
        return true;
    case slang::TypeReflection::ScalarType::Int32:
        outType = CooperativeMatrixComponentType::Int32;
        return true;
    case slang::TypeReflection::ScalarType::UInt32:
        outType = CooperativeMatrixComponentType::Uint32;
        return true;
    default:
        return false;
    }
}

// Returns the CooperativeMatrixScope named by `scopeName` ("subgroup" or "workgroup"), or
// false when the token names no scope.
inline bool tryGetCooperativeMatrixScopeFromName(
    const UnownedStringSlice& scopeName,
    rhi::CooperativeMatrixScope& outScope)
{
    if (scopeName == "subgroup")
    {
        outScope = rhi::CooperativeMatrixScope::Subgroup;
        return true;
    }
    if (scopeName == "workgroup")
    {
        outScope = rhi::CooperativeMatrixScope::Workgroup;
        return true;
    }
    return false;
}

// Parses a -require-coopmat-shape value of the form "<type>:<scope>:<m>:<n>:<k>", for example
// "half:subgroup:8:8:16", into `outDesc`. The shape is a matmul shape because the device
// query matches it against the cooperative-matrix property entries the device lists, and
// each entry describes a full matmul A (m x k) * B (k x n) = C rather than a standalone
// accumulator; a test whose shader only loads and stores an accumulator therefore picks a k
// the device lists alongside the shape's other dimensions (16 for the half-precision
// entries current NVIDIA devices list). The single type token fills all four
// component-type slots, so a mixed-type matmul shape (fp8 inputs with a half accumulator,
// say) cannot be spelled. The match is deliberately conservative: the device query requires
// exact m/n/k equality against fixed property entries and granularity divisibility against
// cooperativeMatrixFlexibleDimensions entries, so a device that lists the same m/n only
// with a different k skips the test even when the shader never multiplies. Returns
// SLANG_FAIL when any token is malformed — including a dimension outside the 1..0xffffffff
// range a CooperativeMatrixDesc can hold — so a typo cannot silently change which devices
// run the test.
inline SlangResult tryParseCooperativeMatrixShape(
    const UnownedStringSlice& value,
    rhi::CooperativeMatrixDesc& outDesc)
{
    List<UnownedStringSlice> tokens;
    StringUtil::split(value, ':', tokens);
    if (tokens.getCount() != 5)
        return SLANG_FAIL;

    rhi::CooperativeMatrixDesc desc = {};
    if (!tryGetCooperativeMatrixComponentTypeFromName(tokens[0], desc.aType))
        return SLANG_FAIL;
    desc.bType = desc.cType = desc.resultType = desc.aType;

    if (!tryGetCooperativeMatrixScopeFromName(tokens[1], desc.scope))
        return SLANG_FAIL;

    Int dims[3];
    for (Index i = 0; i < 3; ++i)
    {
        // The uint32_t casts below would silently wrap a value above the range, gating the
        // test on a different shape than written, so reject it here.
        if (SLANG_FAILED(StringUtil::parseInt(tokens[i + 2], dims[i])) || dims[i] < 1 ||
            dims[i] > (Int)0xffffffff)
        {
            return SLANG_FAIL;
        }
    }
    desc.m = uint32_t(dims[0]);
    desc.n = uint32_t(dims[1]);
    desc.k = uint32_t(dims[2]);

    outDesc = desc;
    return SLANG_OK;
}

} // namespace renderer_test
