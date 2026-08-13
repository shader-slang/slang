// unorm-snorm-non-texture-layout.slang

// `unorm` / `snorm` on a non-texture carrier still has to reach a type layout.
// A structured-buffer element and a struct field both recurse into the modified
// type, which used to fall through to the "unimplemented case in type layout"
// catch-all and crash instead of laying out as the base type (issue #12535).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

struct Norms
{
    unorm float u;
    snorm float s;
}

uniform Norms n;

RWStructuredBuffer<unorm float> inBuf;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = n.u + n.s + inBuf[0];
}

// HLSL keeps the modifier in the emitted spelling, for DXC compatibility.

// HLSL: RWStructuredBuffer<unorm float
// HLSL: unorm float{{.*}}u_0
// HLSL: snorm float{{.*}}s_0

// SPIR-V has no such spelling, so it shows the layout directly: both fields are
// plain floats at consecutive 4-byte offsets, i.e. exactly the layout the base
// type would get, and the buffer element is a plain float.

// SPIRV-DAG: OpMemberDecorate %Norms_std140 0 Offset 0
// SPIRV-DAG: OpMemberDecorate %Norms_std140 1 Offset 4
// SPIRV-DAG: %Norms_std140 = OpTypeStruct %float %float
// SPIRV-DAG: %RWStructuredBuffer = OpTypeStruct %_runtimearr_float
