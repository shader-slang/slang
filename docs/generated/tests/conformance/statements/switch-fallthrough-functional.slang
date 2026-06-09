//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#switch-statement
//META: doc_section_digest=a46f7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6
//META: purpose=A switch case without break falls through to the next case clause.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that omitting break causes execution to fall through to the next case.
// case 0 has no break so it falls through into case 1's body; result reflects both.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)

int fallthrough_test(int v)
{
    int result = 0;
    switch (v)
    {
    case 0:
        result = result + 10;
        // Fall through to case 1
    case 1:
        result = result + 1;
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
    // case 0 falls through: 10 + 1 = 11
    outputBuffer[0] = fallthrough_test(0);
    // case 1 direct: just 1
    outputBuffer[1] = fallthrough_test(1);
    // default: -1
    outputBuffer[2] = fallthrough_test(5);
}

//CHECK: 11
//CHECK: 1
//CHECK: -1
