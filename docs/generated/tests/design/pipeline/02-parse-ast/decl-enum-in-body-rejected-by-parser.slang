//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=35a0377e8327c387000594e690000a91232b821b59188e50e4c846f99b749334
//META: purpose=A `struct` declaration inside a function body parses, but an `enum` is not routed to the declaration path and is rejected with E30102 "enum is not allowed here."
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the statement parser routes only `struct` (and HLSL `class`) into the
// declaration path inside a function body; `enum` falls to the generic
// rejection (it also leaves unexpected-token follow-ups, which are
// deliberately not annotated, hence non-exhaustive). CHECK pins the E30102 code and the kind-specific wording.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int main()
{
    enum E { A = 3 };
//CHECK: ^ error E30102
//CHECK: ^ enum is not allowed here.
    return 0;
}
