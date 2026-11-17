//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:32+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#requirecapabilityatom
//META: doc_section_digest=281884efd5d7d16704b0a8d5fc5f9f82c699d566d7d1b2badab1f5a372822aa7
//META: purpose=requireCapabilityAtom is not produced by lowering a [require(...)] attribute: the user's LOWER-TO-IR module carries none on the entry point, and the decoration appears only in later (layout-IR) module state.
//META: intent=negative
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's callout: `[require(spirv_1_5)]` feeds the entry
// point's inferred capability set but does not itself put a
// requireCapabilityAtom decoration on the function in the user's lowered
// module. The CHECK-NOT is scoped between the LOWER-TO-IR header and the
// entry function; a later dump shows the decoration added from the
// inferred set, so the test is not vacuous.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[require(spirv_1_5)]
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: ### LOWER-TO-IR:
// CHECK-NOT: requireCapabilityAtom
// CHECK: func %main
// CHECK: [requireCapabilityAtom(
