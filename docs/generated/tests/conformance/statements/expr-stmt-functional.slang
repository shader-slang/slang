//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#expression-statement
//META: doc_section_digest=b91a2c3e4f5d6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b
//META: purpose=An expression statement (expression followed by semicolon) is a valid statement form.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an expression statement (function call followed by `;`) executes.
// Also verifies that an assignment expression followed by `;` updates the variable.
//TEST:INTERPRET(filecheck=CHECK):

int computeValue(int x) { return x * 2; }

void main()
{
    int result = 0;
    // Assignment expression statement
    result = computeValue(5);
    // CHECK: 10
    printf("%d\n", result);
}
