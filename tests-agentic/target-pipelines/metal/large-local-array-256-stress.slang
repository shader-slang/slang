//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Stress: a function-local `int arr[256]` survives Phase B/C array-specialization and emits as `array<int, int(256)>` in the kernel.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: large local array. We allocate a 256-element local,
// fill it in a loop, then sample one element with a runtime index
// so the array escapes constant folding.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[256];
    for (uint i = 0; i < 256; ++i)
    {
        arr[i] = int(i);
    }
    outBuf[tid.x] = arr[tid.x % 256];
}

// CHECK: array<int, int(256)>
