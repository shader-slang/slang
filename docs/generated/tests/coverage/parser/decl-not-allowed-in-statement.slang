//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=a90b198ea2328d6f79a99f07405f80975c42e7ee4f6f90cb3a7ef552183ec66f
//META: purpose=Verifies a `namespace` declaration inside a function body is rejected with E30102 ("declaration not allowed here").
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the statement parser allows a restricted set of
// declaration forms; a `namespace` is a file/module-scope-only declaration
// and drives the decl-not-allowed branch when seen as a statement. CHECK
// pins E30102 and the observed message naming the offending declaration kind.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

[numthreads(1, 1, 1)]
void main()
{
    namespace N {}
//CHECK: E30102
//CHECK: namespace is not allowed here.
}
