//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#riff-container-format
//META: doc_section_digest=0c078e337fe0de98e2ce6d13119538d84997c407efe297937c76318154a68096
//META: purpose=The container composes the AST and IR payloads with auxiliary data, and with -embed-downstream-ir a SPIR-V blob joins it and surfaces as an EmbeddedDownstreamIR entry under -dump-module.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the container is a composition rather than a single
// payload: the same .slang-module can hold the module's own IR plus a
// downstream artefact. The test compiles with -target spirv and
// -embed-downstream-ir, then re-reads the file; the CHECKs require both
// the module's own entry-point function and an EmbeddedDownstreamIR
// entry carrying real SPIR-V (the OpCapability line comes from the
// embedded blob's disassembly, not from the module IR), which only
// holds if the extra chunk was written and found again on load.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/embed-downstream-ir-spirv.slang -o docs/generated/tests/design/cross-cutting/serialization/embed-downstream-ir-spirv.slang-module -target spirv -embed-downstream-ir -skip-spirv-validation
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/embed-downstream-ir-spirv.slang-module

module embed_downstream_ir_spirv;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
}

// CHECK: [nameHint("main")]
// CHECK: EmbeddedDownstreamIR({{[0-9]+}} : Int,
// CHECK: OpCapability Shader
