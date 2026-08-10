//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=a90b198ea2328d6f79a99f07405f80975c42e7ee4f6f90cb3a7ef552183ec66f
//META: purpose=Verifies a `__subscript` declaration with no `-> ReturnType` clause is rejected with E30901.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the subscript-declaration parser requires an explicit
// return type introduced by `->`. Omitting it (jumping straight to the
// accessor block) drives the missing-return-type branch. CHECK pins E30901
// and the observed message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

struct S { __subscript(int i) { get { return 0; } } }
//CHECK:              ^ __subscript declaration must have a return type specified after '->'

[numthreads(1, 1, 1)]
void main() {}
