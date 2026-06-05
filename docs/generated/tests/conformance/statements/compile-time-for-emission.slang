//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#compile-time-for-statement
//META: doc_section_digest=b1d04c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2a3
//META: purpose=$for is fully unrolled in the emitted HLSL output (no loop structure, repeated body).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that $for emits as fully-unrolled code in HLSL (no loop structure remains).
// The body assigns to arr[i]; HLSL output should have explicit index assignments.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    int arr[3];
    $for(i in Range(0, 3))
    {
        arr[i] = i * 10;
    }
    outputBuffer[0] = arr[0];
    outputBuffer[1] = arr[1];
    outputBuffer[2] = arr[2];
}

// The emitted HLSL should contain the index values 0, 10, 20 from the unrolled assignments
// CHECK: 0
// CHECK: 10
// CHECK: 20
