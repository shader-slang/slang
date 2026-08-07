//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=9163d71b0ed4735c5c28e2877570515476519666c5a6da87256143a8fab12f94
//META: purpose=Verifies a `private` struct member is not accessible from a free function even within the same source file (E30600).
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the visibility-modifier check on member access treats
// `private` as restricting access to the declaring type's own scope, not
// the whole file. A free function reading a `private` member therefore
// fails accessibility even in the same translation unit. The caret CHECKs
// pin both observed diagnostics emitted at the read site (E30600).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

struct S { private int c; }
RWStructuredBuffer<int> o;

[numthreads(1, 1, 1)]
void main()
{
    S s;
    o[0] = s.c;
//CHECK:     ^ declaration not accessible
//CHECK:     ^ 'c' is not accessible from the current context.
}
