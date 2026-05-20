//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=legalizeArrayReturnType is filtered out for Metal (`!isMetalTarget && !isSPIRV`), so an array-return function survives as `array<T, N> foo(...)` rather than being rewritten to void with an out parameter.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Metal accepts array return types via the `array<T, N>` wrapper,
// so Slang's `legalizeArrayReturnType` pass is excluded from the
// Metal pipeline. We expect to see `array<int, int(4)>` as the
// return type of the helper.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

int[4] makeArr(int v)
{
    int r[4] = { v, v + 1, v + 2, v + 3 };
    return r;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int r[4] = makeArr(int(tid.x));
    outBuf[tid.x] = r[0] + r[1] + r[2] + r[3];
}

// CHECK: array<int, int(4)> makeArr_{{[0-9]+}}
