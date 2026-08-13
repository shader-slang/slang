// unorm-snorm-non-texture-layout.slang

// `unorm` / `snorm` on a non-texture carrier still has to reach a type layout.
// A structured-buffer element and a struct field both recurse into the modified
// type, which used to fall through to the "unimplemented case in type layout"
// catch-all and crash instead of laying out as the base type (issue #12535).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

// `Norms` and `Plain` differ only in the modifiers, so laying a modified type
// out as its base means the two must produce identical layouts.
struct Norms
{
    unorm float u;
    snorm float s;
}

struct Plain
{
    float u;
    float s;
}

uniform Norms n;
uniform Plain p;

// Both modifiers on both carriers: as a buffer element here, as a struct field
// above.
RWStructuredBuffer<unorm float> unormBuf;
RWStructuredBuffer<snorm float> snormBuf;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = n.u + n.s + p.u + p.s + unormBuf[0] + snormBuf[0];
}

// HLSL keeps the modifier in the emitted spelling, for DXC compatibility.

// HLSL-DAG: RWStructuredBuffer<unorm float
// HLSL-DAG: RWStructuredBuffer<snorm float
// HLSL-DAG: unorm float{{.*}}u_0
// HLSL-DAG: snorm float{{.*}}s_0

// SPIR-V has no such spelling, so it shows the layout directly. Both structs
// lower to the same member types and the same offsets, which is the property
// this fix establishes; the buffer element is a plain float array.

// SPIRV-DAG: %Norms_std140 = OpTypeStruct %float %float
// SPIRV-DAG: %Plain_std140 = OpTypeStruct %float %float
// SPIRV-DAG: OpMemberDecorate %Norms_std140 0 Offset 0
// SPIRV-DAG: OpMemberDecorate %Norms_std140 1 Offset 4
// SPIRV-DAG: OpMemberDecorate %Plain_std140 0 Offset 0
// SPIRV-DAG: OpMemberDecorate %Plain_std140 1 Offset 4
// SPIRV-DAG: %RWStructuredBuffer = OpTypeStruct %_runtimearr_float
