//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/cross-cutting/serialization.md#riff-container-format
//META: doc_section_digest=28458173cf644619b7cdf4174e4c18bea97d5fb11457cbbeeca752dd510622c7
//META: purpose=Container packaging composes AST + IR + auxiliary data; with -embed-downstream-ir on -target dxil a DXIL blob joins the container and surfaces as an EmbeddedDownstreamIR(10 : Int, ...) marker under -dump-module.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Companion to `embed-downstream-ir-spirv.slang` — verifies that the
// "container packages a downstream-IR blob" claim holds for the DXIL
// downstream target, not just SPIR-V. `-dump-module` surfaces the
// embedded DXIL blob with `EmbeddedDownstreamIR(10 : Int, ...)`
// (10 is the SLANG_DXIL target enum from `include/slang.h`); the
// DXIL payload itself dumps as `<binary blob>` because the IR
// dumper only disassembles SPIR-V. Pinning the target-enum integer
// is what distinguishes a DXIL embedding from a SPIR-V one. CI runs
// DXC; the agent runtime that generated this test does not.

//TEST:COMPILE: tests-agentic/cross-cutting/serialization/embed-downstream-ir-dxil.slang -o tests-agentic/cross-cutting/serialization/embed-downstream-ir-dxil.slang-module -target dxil -embed-downstream-ir -profile lib_6_6 -incomplete-library
//TEST:SIMPLE(filecheck=CHECK): -dump-module tests-agentic/cross-cutting/serialization/embed-downstream-ir-dxil.slang-module

module embed_downstream_ir_dxil;

public float4 addVectors(float4 a, float4 b)
{
    return a + b;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
}

// CHECK: EmbeddedDownstreamIR(10 : Int,
// CHECK: <binary blob>
