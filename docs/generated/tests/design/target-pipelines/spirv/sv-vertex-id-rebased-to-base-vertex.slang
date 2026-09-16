//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=f8f9c6c9ed941a5154d83b06809c18c71fb5d69313dfbb7b6096dcb44af0485e
//META: purpose=A load of an `SV_VertexID` parameter is rewritten arithmetically to `gl_VertexIndex - gl_BaseVertex`, emitting an `OpISub` of two builtin loads and pulling in the `DrawParameters` capability.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that SV_VertexID is not merely renamed to a Vulkan builtin but
// rebased: HLSL's vertex index is zero-based per draw while Vulkan's includes
// the draw's base offset, so the pass subtracts the base to keep user code on
// HLSL semantics. The CHECKs pin both builtin identities, the subtraction
// that combines them, and the capability the BaseVertex operand drags in --
// a test that only looked for VertexIndex would pass without the rebasing.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage vertex

[shader("vertex")]
float4 main(uint vid : SV_VertexID) : SV_Position
{
    return float4(float(vid), 0, 0, 1);
}

// CHECK-DAG: OpCapability DrawParameters
// CHECK-DAG: BuiltIn BaseVertex
// CHECK-DAG: BuiltIn VertexIndex
// CHECK: OpISub %int
