//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=a90b198ea2328d6f79a99f07405f80975c42e7ee4f6f90cb3a7ef552183ec66f
//META: purpose=Verifies an integer literal with an unrecognized suffix character is rejected with the invalid-suffix diagnostic.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the integer-literal parser recognizes only specific
// suffix characters (u/l etc.); a stray letter such as `q` drives the
// invalid-suffix branch. CHECK pins the diagnostic code and the observed
// message naming the rejected suffix.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

static const int x = 5q;
//CHECK: E39999
//CHECK: invalid suffix 'q' on integer literal

[numthreads(1, 1, 1)]
void main() {}
