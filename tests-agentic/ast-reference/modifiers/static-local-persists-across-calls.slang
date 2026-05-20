//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#compatibility-and-hlsl-storage-class-modifiers
//META: doc_section_digest=0b146a7577c216d587114be607b7bee424fa867d45b9300854bbc44543473939
//META: purpose=A function-scope `static` variable retains its value across calls to the function.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLStaticModifier` makes a function-scope variable persist across
// calls (analogous to C's function-static). The first call initializes
// the storage; subsequent calls observe the prior modification. The
// interpreter rejects non-const static globals, so we observe on CPU.

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
