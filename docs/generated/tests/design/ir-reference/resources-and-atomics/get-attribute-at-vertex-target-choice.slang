//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=212a9e3fd4ca324be91b034872f2b7a9afdb1fe8ac5acfc9f068664d69c3e326
//META: purpose=`GetAttributeAtVertex(attribute, vertexIndex)` on a `nointerpolation` fragment input is the single user surface behind `ResolveVaryingInputRef` and `GetPerVertexInputArray`, and which of the two you get is a target decision: HLSL re-emits its own `GetAttributeAtVertex`, while GLSL and SPIR-V subscript a per-vertex input array.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the two Shader-IO opcodes share one surface and that the
// target, not the source text, picks between them.
//
// The `-dump-ir` snapshot is not the right observation point here: the
// core-module function's `__target_switch` body is pulled into the very
// first dump for every target, so both opcodes are visible on all three
// and the dump cannot distinguish them. The target-specific arm is only
// settled once the switch is specialized, so this test pins the final
// emitted text instead. HLSL keeps a scalar attribute and re-emits the
// intrinsic by name (the `ResolveVaryingInputRef`-only arm); GLSL declares
// the input as a `pervertexEXT` array of three and subscripts index 1, and
// SPIR-V does the same through a `PerVertexKHR`-decorated array-typed
// Input variable (the arm that wraps the reference in
// `GetPerVertexInputArray` before subscripting it).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage fragment -entry main

struct VOut
{
    float4 pos : SV_Position;
    nointerpolation float3 color : COLOR;
};

float4 main(VOut input) : SV_Target
{
    return float4(GetAttributeAtVertex(input.color, 1), 1.0);
}

// HLSL: GetAttributeAtVertex(

// GLSL-DAG: pervertexEXT
// GLSL-DAG: in vec3  {{[A-Za-z0-9_]+}}[3]

// SPV-DAG: OpDecorate %{{[A-Za-z0-9_]+}} PerVertexKHR
// SPV-DAG: OpTypeArray %v3float
