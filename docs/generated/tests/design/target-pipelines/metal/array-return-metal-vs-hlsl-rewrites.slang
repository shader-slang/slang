//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=Cross-target negative observation: Metal keeps an array-return function as `array<T, N> foo(...)` (legalizeArrayReturnType skipped), while HLSL rewrites the function to `void` with an `out T[N]` parameter.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Demonstrates the "Metal-does-X, HLSL-does-Y" split for
// `legalizeArrayReturnType`. Same source, two targets, distinct
// CHECK prefixes.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl  -entry main -stage compute

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
// HLSL: void makeArr_{{[0-9]+}}
// HLSL-SAME: out
