//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#for-statement
//META: doc_section_digest=b57a8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7
//META: purpose=A variable declared in the for-loop init has scope limited to the for statement.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the for-loop init variable is scoped to the for statement.
// Two successive for loops may redeclare the same variable name `i` without conflict.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    int sum1 = 0;
    for (int i = 0; i < 3; i++)
        sum1 += i;

    int sum2 = 0;
    for (int i = 0; i < 5; i++)  // same name `i` is valid because previous scope ended
        sum2 += i;

    outputBuffer[0] = sum1;  // 0+1+2 = 3
    outputBuffer[1] = sum2;  // 0+1+2+3+4 = 10
}

// CHECK: void computeMain
