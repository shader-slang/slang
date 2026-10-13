//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#context-predicates
//META: doc_section_digest=666232460cc1549150d42e63540887426056c0a1dd5950548f0704f2d865055e
//META: purpose=Phase D returns early when an earlier diagnostic raised an error, so the validators after checkForRecursiveTypes never run on the same module.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the error-count predicate that ends Phase D early. The module
// contains two independently diagnosable problems: a recursive struct that
// D1 rejects, and a non-void function missing a return that D5 would warn
// about. Because D1 raised an error first, Phase D bails out and the
// missing-return warning is never produced -- so annotating only the
// cyclic-reference error and running the matcher in exhaustive mode is
// what proves the later validator was skipped.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

struct Bad { Bad next; };
/*CHECK:
       ^^^ type contains cyclic reference
       ^^^ type 'Bad' contains cyclic reference to itself.
*/
// CHECK: E41001

int helper(int x) { if (x > 0) return 1; }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helper(a);
}
