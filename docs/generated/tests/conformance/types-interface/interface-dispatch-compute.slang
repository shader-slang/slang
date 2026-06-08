//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=Interface-typed function parameter dispatch produces correct results on the CPU compute backend; the same code calling two different concrete types via an interface parameter produces distinct correct outputs.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Exercise interface dispatch through COMPARE_COMPUTE CPU backend.
// A compute kernel calls an interface-parameterized function with a
// concrete type and writes the result to an output buffer.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IScalar
{
    int compute(int x);
}

struct Doubler : IScalar
{
    int compute(int x) { return x * 2; }
}

int dispatchCompute(IScalar s, int x)
{
    return s.compute(x);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Doubler d;
    outputBuffer[0] = dispatchCompute(d, 5);  // 10
}

//CHECK: 10
