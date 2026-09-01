//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=557b5025d6a79f84b833cef681013d91b8a66b0574857ec5bf62d3232bf54ca2
//META: purpose=In SPIR-V mode the float-non-uniform-resource-index pass propagates a `NonUniform` decoration onto the indexed resource access.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the SPIR-V half of the pass's documented two-mode behaviour:
// the `NonUniformResourceIndex` wrapper is bubbled to the leaf and a
// `NonUniform` decoration is emitted. A resource array is indexed with a
// thread-varying index so the wrapper survives constant folding; the
// CHECK asserts both the `ShaderNonUniform` capability and at least one
// `OpDecorate ... NonUniform`, which only appear when the decoration is
// propagated rather than merely repositioned.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform RWStructuredBuffer<int> outBuf;
uniform StructuredBuffer<int> tables[8];

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = tables[NonUniformResourceIndex(tid.x)][0];
}

// SPIRV-DAG: OpCapability ShaderNonUniform
// SPIRV-DAG: OpDecorate %{{[A-Za-z0-9_]+}} NonUniform
