//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=c7f55cb42aaa9e03df513fb2e3ff5bbcf380f2c09432d750a2aa9c55dfc56801
//META: purpose=The NullPtrType of the `nullptr` literal is accepted at a pointer-typed target without an explicit cast and compares equal to a null pointer.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `nullptr` has its own type which converts to `T*`: an
// `int*` local is initialised from the bare literal and then compared
// against it. Both the comparison result and a comparison against a
// real pointee address are written to a buffer; CHECK asserts 1 then 0,
// so a `nullptr` that silently became a valid address would fail.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set src = ubuffer(data=[7], stride=4)
uniform int* src;
//TEST_INPUT: set outBuf = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    int* p = nullptr;
    // CHECK: 1
    outBuf[0] = (p == nullptr) ? 1 : 0;
    p = src;
    // CHECK: 0
    outBuf[1] = (p == nullptr) ? 1 : 0;
}
