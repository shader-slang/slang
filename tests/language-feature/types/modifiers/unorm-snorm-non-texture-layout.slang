// unorm-snorm-non-texture-layout.slang

// `unorm` / `snorm` on a non-texture carrier still has to reach a type layout.
// A structured-buffer element and a struct field both recurse into the modified
// type, which used to fall through to the "unimplemented case in type layout"
// catch-all and crash instead of laying out as the base type (issue #12535).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

// `Norms` and `Plain` differ only in the modifiers, so laying a modified type
// out as its base means the two must produce identical layouts.
// `no_diff` is the third type modifier and takes the same path, so it belongs
// in the same comparison rather than being assumed to work.
struct Norms
{
    unorm float u;
    snorm float s;
    no_diff float d;
}

struct Plain
{
    float u;
    float s;
    float d;
}

// A vector carrier too: under std140 a float4 has 16-byte alignment, so this
// catches a size/alignment mistake the scalar case would not.
struct NormsVec
{
    unorm float4 v;
}

struct PlainVec
{
    float4 v;
}

uniform Norms n;
uniform Plain p;
uniform NormsVec nv;
uniform PlainVec pv;

// Both modifiers on both carriers: as a buffer element here, as a struct field
// above.
RWStructuredBuffer<unorm float> unormBuf;
RWStructuredBuffer<snorm float> snormBuf;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = n.u + n.s + n.d + p.u + p.s + p.d + nv.v.x + pv.v.x + unormBuf[0] +
                      snormBuf[0];
}

// The HLSL half only checks that the modifier survives into the emitted
// spelling, for DXC compatibility. The layout-equality claim is checked on the
// SPIR-V side below, which is where layout is observable.

// HLSL-DAG: RWStructuredBuffer<unorm float
// HLSL-DAG: RWStructuredBuffer<snorm float
// HLSL-DAG: unorm float{{.*}}u_0
// HLSL-DAG: snorm float{{.*}}s_0

// SPIR-V has no such spelling, so it shows the layout directly. Each modified
// struct lowers to the same member types at the same offsets as its plain twin,
// which is the property this fix establishes.

// SPIRV-DAG: %NormsVec_std140 = OpTypeStruct %v4float
// SPIRV-DAG: %PlainVec_std140 = OpTypeStruct %v4float

// The four uniform structs sit at 0/16/32/48, so `NormsVec` occupies the same
// 16 bytes `PlainVec` does -- the alignment-sensitive half of the claim.

// SPIRV-DAG: OpMemberDecorate %GlobalParams_std140 2 Offset 32
// SPIRV-DAG: OpMemberDecorate %GlobalParams_std140 3 Offset 48

// `calcSizeAndAlignment` is the IR-side half of this fix, and its result is the
// runtime-array stride, so pin that directly rather than only the element type
// (which comes from emit unwrapping the attribute, not from the layout code).

// SPIRV-DAG: OpDecorate %_runtimearr_float ArrayStride 4

// The unorm / snorm / no_diff triple lays out as three bare floats, at the same
// offsets its unmodified twin gets.

// SPIRV-DAG: %Norms_std140 = OpTypeStruct %float %float %float{{$}}
// SPIRV-DAG: %Plain_std140 = OpTypeStruct %float %float %float{{$}}
// SPIRV-DAG: OpMemberDecorate %Norms_std140 0 Offset 0
// SPIRV-DAG: OpMemberDecorate %Norms_std140 1 Offset 4
// SPIRV-DAG: OpMemberDecorate %Norms_std140 2 Offset 8
// SPIRV-DAG: OpMemberDecorate %Plain_std140 0 Offset 0
// SPIRV-DAG: OpMemberDecorate %Plain_std140 1 Offset 4
// SPIRV-DAG: OpMemberDecorate %Plain_std140 2 Offset 8
// SPIRV-DAG: %RWStructuredBuffer = OpTypeStruct %_runtimearr_float
