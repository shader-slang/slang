//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:02Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=d2933f044671826cdda47d2bf6584eb3ef9b48c4c8f5e77a17b011c61c8729d4
//META: purpose=A modern-style parameter written without `':' Type` and without an initializer parses but is rejected by the checker with E30621.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the checker rejects `func f(x)`: the parser accepts a bare name
// as a modern parameter, but with no type and no initializer to derive one from
// the checker reports E30621. CHECK asserts the code so an unrelated error
// cannot satisfy the test.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -stage compute -entry main

func f(x) -> int { return 1; }

[numthreads(1, 1, 1)]
void main() {}

// CHECK: E30621
