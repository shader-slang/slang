//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
//META: purpose=Verifies the int MAX literal 2147483647 is emitted as int(2147483647) on CUDA.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: signed integer MAX literal in CUDA emit.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = 2147483647;
    outBuf[tid.x] = a + int(tid.x);
}

// CHECK: int(2147483647)
