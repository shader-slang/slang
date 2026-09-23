//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=babb40e44ba0eb7addfafeddc37185c8c3bbed4c27b3b7fc8a2469d95b33aca1
//META: purpose=Negative - [numthreads()] with zero arguments fires E31001 wrong-argument-count, before any numThreads decoration is created.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the arity floor of the `numthreads` attribute. One argument is
// accepted (see the x-only sibling) but zero is not, so this pins the lower
// bound of the accepted forms. The caret annotation ties E31001 to the
// attribute span, showing the check happens during semantic checking rather
// than at lowering.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main
uniform RWStructuredBuffer<int> buf;

[numthreads()]
/*CHECK:
 ^^^^^^^^^^ wrong number of attribute arguments
*/
// CHECK: E31001
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}
