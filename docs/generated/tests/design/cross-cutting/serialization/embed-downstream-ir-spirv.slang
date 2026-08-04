//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:14:10Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#riff-container-format
//META: doc_section_digest=bfd9e20f8387a0e2ddadf7446a8bed2df7373cdba7ea1ede802fc7577b3e1f2e
//META: purpose=Container packaging composes AST + IR + auxiliary data; with -embed-downstream-ir a SPIR-V blob joins the container and surfaces as an EmbeddedDownstreamIR marker under -dump-module.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "RIFF container format" describes how container orchestration
// composes AST + IR + auxiliary payloads (e.g. an embedded downstream-
// IR blob) into one chunked file with well-known chunk codes. The
// user-facing observable is that when `-embed-downstream-ir` is set,
// the container gains a downstream-IR chunk; `-dump-module` surfaces
// it with an `EmbeddedDownstreamIR(...)` marker.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/embed-downstream-ir-spirv.slang -o docs/generated/tests/design/cross-cutting/serialization/embed-downstream-ir-spirv.slang-module -target spirv -embed-downstream-ir -skip-spirv-validation
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/embed-downstream-ir-spirv.slang-module

module embed_downstream_ir_spirv;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
}

// CHECK: EmbeddedDownstreamIR
// CHECK: OpCapability
