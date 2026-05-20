//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:14:10Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/cross-cutting/serialization.md#riff-container-format
//META: doc_section_digest=28458173cf644619b7cdf4174e4c18bea97d5fb11457cbbeeca752dd510622c7
//META: purpose=Negative control to the -embed-downstream-ir claim: without the flag, the container does not carry a downstream-IR chunk and -dump-module shows no EmbeddedDownstreamIR marker.
//META: intent=regression
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pair with `embed-downstream-ir-spirv.slang`: without
// `-embed-downstream-ir`, the container omits the downstream-IR
// chunk. The IR-module payload still records the function symbol
// (which we check), but the EmbeddedDownstreamIR marker is absent.

//TEST:COMPILE: tests-agentic/cross-cutting/serialization/no-embed-downstream-ir-absent.slang -o tests-agentic/cross-cutting/serialization/no-embed-downstream-ir-absent.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module tests-agentic/cross-cutting/serialization/no-embed-downstream-ir-absent.slang-module

module no_embed_downstream_ir_absent;

public int cube(int x)
{
    return x * x * x;
}

// CHECK: [public]
// CHECK: [nameHint("cube")]
// CHECK-NOT: EmbeddedDownstreamIR
