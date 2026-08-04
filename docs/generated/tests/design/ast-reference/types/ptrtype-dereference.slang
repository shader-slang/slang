//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=A PtrType `T*` is a raw pointer whose pointee is reachable by dereference; a load through the pointer reads the pointed-to value and a store writes it back.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies PtrType `T*` behaves as the raw-pointer type: the pointee
// is read via `*p` and written via `*p = ...`. A uniform int* is fed
// with value 5, dereferenced, mutated, and re-read into an output
// buffer. CHECK asserts both the original and the updated pointee.

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
