//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#riff-container-format
//META: doc_section_digest=0c078e337fe0de98e2ce6d13119538d84997c407efe297937c76318154a68096
//META: purpose=Without -embed-downstream-ir the container carries only the AST and IR payloads, so -dump-module reports the module's own IR and no EmbeddedDownstreamIR entry.
//META: intent=regression
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The negative control for embed-downstream-ir-spirv.slang, and the
// reason that test's marker is meaningful: the container composes
// whatever payloads the compile produced, so with no downstream target
// requested nothing downstream should be attached. Compiling this
// module with a plain `-o` and re-reading it must still show the
// module's own function IR, while CHECK-NOT asserts that no
// EmbeddedDownstreamIR entry appears anywhere in the dump.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/no-embed-downstream-ir-absent.slang -o docs/generated/tests/design/cross-cutting/serialization/no-embed-downstream-ir-absent.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/no-embed-downstream-ir-absent.slang-module

module no_embed_downstream_ir_absent;

public int cube(int x)
{
    return x * x * x;
}

// CHECK: [nameHint("cube")]
// CHECK-NOT: EmbeddedDownstreamIR
