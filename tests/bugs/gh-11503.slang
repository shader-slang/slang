// Regression coverage for shader-slang/slang#11503 — schema-aligned encoding
// for the synthesized `format` operand on `IRTextureType`.
//
// resolveTextureFormatForParameter (slang-ir-resolve-texture-format.cpp)
// synthesizes a fresh integer constant for the `format` operand whenever an
// `[format(...)]` decoration disagrees with the type-token's existing format.
// The schema declares the operand as signed `int` (hlsl.meta.slang `let
// format:int`); the canonical encoding used by sibling producers
// (slang-ir-util.cpp, slang-emit-spirv.cpp) is `getIntType()`. A `uint`
// encoding here would fragment the IR-builder's hoistable-constant cache
// against an `int`-encoded peer of the same logical value (e.g. the
// `!hasFormat()` fallbacks slated to land in #11499).

//TEST:SIMPLE(filecheck=IR): -target spirv-asm -entry computeMain -stage compute -dump-ir-after resolveTextureFormat

[format("r16f")]
RWTexture2D<float> rwt2D;

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    rwt2D[uint2(idx, idx)] = (float)idx;
    outBuf[idx] = (float)idx;
}

// IR: ### AFTER resolveTextureFormat
// IR: TextureType({{.*}}, {{[0-9]+}} : Int){{.*}}global_param
// IR-NOT: TextureType({{.*}}UInt{{.*}}global_param
