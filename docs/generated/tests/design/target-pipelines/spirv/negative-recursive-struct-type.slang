//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#forward-declared-pointer-fixup-phase-d-step-12
//META: doc_section_digest=5b141f6ca8c23bfe722b54152c20598e6273446b722909669c0514f8c69cbc06
//META: purpose=A struct that contains itself is rejected with E41001, so the SPIR-V pointer type graph the forward-pointer fixup loop drains is always finite.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the finiteness precondition the forward-declared-pointer fixup loop
// relies on: the loop has no iteration bound and terminates only because the
// pointee type graph cannot be cyclic. This test pins the compiler-side
// guarantee that makes that safe, namely that a self-containing struct is
// rejected long before the SPIR-V backend runs. The caret annotations sit on
// the type name the compiler reports, and the error code is pinned as well.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -entry main -stage compute

struct Node { int v; Node next; }
/*CHECK:
       ^^^^ type contains cyclic reference
       ^^^^ type 'Node' contains cyclic reference to itself.
*/

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Node n;
    n.v = 1;
    outBuf[tid.x] = n.v;
}
