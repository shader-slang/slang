//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#switch-statement
//META: doc_section_digest=a46f7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6
//META: purpose=switch routes to the matching case label; default fires when no case matches.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that switch dispatches to the matching case and that default fires for no-match.
// Uses COMPARE_COMPUTE -cpu because switch inside a helper function crashes slangi (bug).
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)

int describe(int v)
{
    int result = 0;
    switch (v)
    {
    case 0:
        result = 100;
        break;
    case 1:
        result = 200;
        break;
    default:
        result = -1;
        break;
    }
    return result;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = describe(0);
    outputBuffer[1] = describe(1);
    outputBuffer[2] = describe(99);
}

//CHECK: 100
//CHECK: 200
//CHECK: -1
