//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#return-statement
//META: doc_section_digest=a0cf3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2
//META: purpose=return; (with no value) is valid in a void function and immediately exits it.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `return;` in a void function exits it immediately.
// Code after the return is not executed; observable via a counter.
//TEST:INTERPRET(filecheck=CHECK):

void earlyExit(int x, inout int counter)
{
    counter++;
    if (x == 0)
        return;
    counter++; // only reached when x != 0
}

void main()
{
    int c1 = 0;
    earlyExit(0, c1);
    // returned early: counter is 1 (incremented once before return)
    // CHECK: 1
    printf("%d\n", c1);

    int c2 = 0;
    earlyExit(1, c2);
    // did not return early: counter is 2
    // CHECK: 2
    printf("%d\n", c2);
}
