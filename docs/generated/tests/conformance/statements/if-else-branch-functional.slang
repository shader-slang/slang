//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#if-statement
//META: doc_section_digest=f35e6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5
//META: purpose=if(cond) stmt else stmt executes the else-branch when cond is false.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the else clause: when condition is false, the else-branch executes.
// Tests both true (then) and false (else) paths of a single if-else.
//TEST:INTERPRET(filecheck=CHECK):

int classify(int n)
{
    if (n > 0)
        return 1;
    else
        return -1;
}

void main()
{
    // CHECK: 1
    printf("%d\n", classify(5));
    // CHECK: -1
    printf("%d\n", classify(-3));
}
