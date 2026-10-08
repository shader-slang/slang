//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compatibility-and-hlsl-storage-class-modifiers
//META: doc_section_digest=5bcb788bb16e79fc81babc467bf58625410f93828b43732190bf907b4e20e726
//META: purpose=A function-scope `static` variable retains its value across calls to the function.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `static` inside a function gives the variable storage that outlives
// one activation instead of re-initialising per call. `counter()` is called three
// times from one thread and CHECK asserts the buffer holds 1, 2, 3 -- a per-call
// local would produce 1, 1, 1. The interpreter rejects non-const static globals,
// so the observation runs on the CPU target.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outBuf
RWStructuredBuffer<int> outBuf;

int counter()
{
    static int n = 0;
    n = n + 1;
    return n;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = counter();
    outBuf[1] = counter();
    outBuf[2] = counter();
}
//CHECK: 1
//CHECK: 2
//CHECK: 3
