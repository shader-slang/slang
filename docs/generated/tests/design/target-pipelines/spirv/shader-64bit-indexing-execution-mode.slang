//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#capabilities-and-extensions-decided-at-emit-time
//META: doc_section_digest=a1bda696e594d0d008416384d3cd2b7b449a57153c72f78ceef01aa513a35c5e
//META: purpose=A [Shader64BitIndexing] entry point declares SPV_EXT_shader_64bit_indexing, the Shader64BitIndexingEXT capability, and the matching execution mode.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the three-part emit consequence of the opt-in 64-bit-indexing
// attribute: an extension declaration, a capability, and an execution mode on
// the entry point. The attribute is applied directly to the compute entry
// point; CHECK asserts all three tokens appear, which together are what a
// driver needs to accept 64-bit indices.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
[Shader64BitIndexing]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK-DAG: OpCapability Shader64BitIndexingEXT
// CHECK-DAG: OpExtension "SPV_EXT_shader_64bit_indexing"
// CHECK-DAG: OpExecutionMode %{{[a-zA-Z_0-9]+}} Shader64BitIndexingEXT
