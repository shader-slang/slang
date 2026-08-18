//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=6e338f870e1f336f86d9ce297f0955e56a8c161eb5a129e60e2a396aa3aa18b3
//META: purpose=Expansion: a local fixed-size array declaration is emitted as `T name[N];` in HLSL.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The shared CLikeSourceEmitter writes fixed-size array
// declarations with HLSL-style trailing brackets.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[4];
    arr[0] = int(tid.x);
    arr[1] = int(tid.x) + 1;
    arr[2] = int(tid.x) + 2;
    arr[3] = int(tid.x) + 3;
    outBuf[tid.x] = arr[2];
}

// CHECK: int arr_{{[0-9]+}}[
