//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=c7f55cb42aaa9e03df513fb2e3ff5bbcf380f2c09432d750a2aa9c55dfc56801
//META: purpose=A PtrType `T*` is the raw-pointer type whose pointee is reachable by dereference; a load through the pointer reads the pointed-to value and a store writes it back.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `T*` behaves as a raw pointer with a mutable pointee.
// A uniform int* is fed a buffer holding 5, read through `*p`, written
// through `*p = 100`, then re-read. CHECK asserts 5 then 100, so a
// pointer that copied its pointee instead of aliasing it would fail the
// second observation.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set p = ubuffer(data=[5], stride=4)
uniform int* p;
//TEST_INPUT: set outBuf = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 5
    outBuf[0] = *p;
    *p = 100;
    // CHECK: 100
    outBuf[1] = *p;
}
