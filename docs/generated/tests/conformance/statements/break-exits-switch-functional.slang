//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#break-statement
//META: doc_section_digest=e8ad1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0
//META: purpose=break transfers control to after the end of the immediately enclosing switch statement.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that break in a switch body exits the switch and continues after it.
// After the switch, the variable is checked to confirm control transferred correctly.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 0;
    switch (5)
    {
    case 5:
        x = 1;
        break;
        x = 99; // unreachable
    default:
        x = -1;
        break;
    }
    // Only case 5 body ran; break exited; unreachable code not reached
    // CHECK: 1
    printf("%d\n", x);
}
