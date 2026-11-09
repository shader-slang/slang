//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=f8f9c6c9ed941a5154d83b06809c18c71fb5d69313dfbb7b6096dcb44af0485e
//META: purpose=A load of an `SV_InstanceID` parameter is rewritten to `gl_InstanceIndex - gl_BaseInstance`, the instance-side companion of the vertex rebasing.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second of the two semantics with no 1:1 Vulkan builtin. It is
// a distinct row from SV_VertexID because it names different builtins, so
// covering only one would leave the other's mapping unchecked. The CHECKs
// pin the instance-side builtin pair plus the subtraction, and the shared
// DrawParameters capability that both base-offset builtins require.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage vertex

[shader("vertex")]
float4 main(uint iid : SV_InstanceID) : SV_Position
{
    return float4(float(iid), 0, 0, 1);
}

// CHECK-DAG: OpCapability DrawParameters
// CHECK-DAG: BuiltIn BaseInstance
// CHECK-DAG: BuiltIn InstanceIndex
// CHECK: OpISub %int
