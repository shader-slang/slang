// Regression coverage for shader-slang/slang#11503.
//
// The `IRTextureType` schema in hlsl.meta.slang declares the `format`
// operand as signed `int`; the synthesized constant must dump as `Int`,
// not `UInt`, to keep the IR-builder's hoistable-constant cache key
// aligned with sibling producers.

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
// IR: TextureType({{.*}}, {{[0-9]+}} : Int){{.*}}= global_param
// IR-NOT: TextureType({{.*}}, {{[0-9]+}} : UInt){{.*}}= global_param
