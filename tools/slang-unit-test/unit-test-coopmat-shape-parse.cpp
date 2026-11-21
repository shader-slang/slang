// unit-test-coopmat-shape-parse.cpp
//
// Contract under test
// -------------------
// tryParseCooperativeMatrixShape parses the value of render-test's
// -require-coopmat-shape option: "<type>:<scope>:<m>:<n>:<k>". That option is the
// gate that lets a cooperative-matrix test skip on devices that do not list the
// shapes its shader declares, so a silent parse regression would mis-gate tests:
// every malformed input must fail, and a value the parse accepts must land in the
// CooperativeMatrixDesc unchanged.
//
// What this suite pins:
//
//   * the happy path maps the shader's canonical type spellings
//     (resolved through TypeTextUtil::findScalarType) and the
//     subgroup/workgroup scopes into the desc, with the single type token
//     filling all four component-type slots;
//   * every malformed token fails: wrong token count, an unknown type (both
//     a typo and a real type with no cooperative-matrix component type), a
//     non-canonical type spelling, an unknown scope, and non-numeric /
//     zero / negative / out-of-uint32-range dimensions;
//   * dimensions at and just below the uint32 boundary are accepted, while
//     values above it fail rather than wrapping to a different shape.

#include "coopmat-shape-parse.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using namespace renderer_test;

namespace
{

SlangResult parse(const char* value, rhi::CooperativeMatrixDesc& outDesc)
{
    return tryParseCooperativeMatrixShape(UnownedStringSlice(value), outDesc);
}

bool isHalf(const rhi::CooperativeMatrixDesc& desc)
{
    return desc.aType == rhi::CooperativeMatrixComponentType::Float16 &&
           desc.bType == rhi::CooperativeMatrixComponentType::Float16 &&
           desc.cType == rhi::CooperativeMatrixComponentType::Float16 &&
           desc.resultType == rhi::CooperativeMatrixComponentType::Float16;
}

} // namespace

SLANG_UNIT_TEST(coopMatShapeParseHappyPath)
{
    rhi::CooperativeMatrixDesc desc;
    SLANG_CHECK(SLANG_SUCCEEDED(parse("half:subgroup:8:8:16", desc)));
    SLANG_CHECK(desc.m == 8 && desc.n == 8 && desc.k == 16);
    SLANG_CHECK(isHalf(desc));
    SLANG_CHECK(desc.scope == rhi::CooperativeMatrixScope::Subgroup);
}

SLANG_UNIT_TEST(coopMatShapeParseCanonicalTypeSpellings)
{
    rhi::CooperativeMatrixDesc desc;
    // The accepted spellings are the shader's type names (TypeTextUtil::findScalarType),
    // so a test author can copy the component type out of the CoopMat declaration.
    SLANG_CHECK(SLANG_SUCCEEDED(parse("int8_t:workgroup:16:16:32", desc)));
    SLANG_CHECK(desc.aType == rhi::CooperativeMatrixComponentType::Int8);
    SLANG_CHECK(desc.scope == rhi::CooperativeMatrixScope::Workgroup);

    SLANG_CHECK(SLANG_SUCCEEDED(parse("uint16_t:subgroup:16:16:16", desc)));
    SLANG_CHECK(desc.aType == rhi::CooperativeMatrixComponentType::Uint16);

    SLANG_CHECK(SLANG_SUCCEEDED(parse("bfloat16:subgroup:16:16:16", desc)));
    SLANG_CHECK(desc.aType == rhi::CooperativeMatrixComponentType::Bfloat16);

    SLANG_CHECK(SLANG_SUCCEEDED(parse("float_e4m3:subgroup:16:16:32", desc)));
    SLANG_CHECK(desc.aType == rhi::CooperativeMatrixComponentType::FloatE4M3);

    SLANG_CHECK(SLANG_SUCCEEDED(parse("float_e5m2:subgroup:16:16:32", desc)));
    SLANG_CHECK(desc.aType == rhi::CooperativeMatrixComponentType::FloatE5M2);
}

SLANG_UNIT_TEST(coopMatShapeParseRejectsMalformedValues)
{
    rhi::CooperativeMatrixDesc desc;
    SLANG_CHECK(SLANG_FAILED(parse("half:subgroup:8:8", desc)));      // four tokens
    SLANG_CHECK(SLANG_FAILED(parse("half:subgroup:8:8:16:0", desc))); // six tokens
    SLANG_CHECK(SLANG_FAILED(parse("halve:subgroup:8:8:16", desc)));  // unknown type
    SLANG_CHECK(SLANG_FAILED(parse("int8:subgroup:8:8:16", desc)));   // non-canonical spelling
    SLANG_CHECK(SLANG_FAILED(parse("double:subgroup:8:8:16", desc))); // no coopmat component type
    SLANG_CHECK(SLANG_FAILED(parse("half:cluster:8:8:16", desc)));    // unknown scope
    SLANG_CHECK(SLANG_FAILED(parse("half:subgroup:0:8:16", desc)));   // zero dimension
    SLANG_CHECK(SLANG_FAILED(parse("half:subgroup:8:-8:16", desc)));  // negative dimension
    SLANG_CHECK(SLANG_FAILED(parse("half:subgroup:8:8:x", desc)));    // non-numeric dimension
}

SLANG_UNIT_TEST(coopMatShapeParseRejectsDimensionsAboveUint32)
{
    rhi::CooperativeMatrixDesc desc;
    // Values above the uint32 range must fail rather than wrap: 2^32 + 64 would
    // truncate to 64 and silently gate the test on a different shape.
    SLANG_CHECK(SLANG_FAILED(parse("half:subgroup:8:8:4294967360", desc)));
    // Values at and just below the boundary still parse.
    SLANG_CHECK(SLANG_SUCCEEDED(parse("half:subgroup:8:8:4294967295", desc)));
    SLANG_CHECK(desc.k == 0xffffffffu);
}
