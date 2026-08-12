//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#return-statement
//META: doc_section_digest=a0cf3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2
//META: purpose=The returned value is coerced to the function's declared result type.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an int literal returned from a float-typed function is coerced to float.
// The caller receives a float value and can compare it.
//TEST:INTERPRET(filecheck=CHECK):

float toFloat() { return 3; }  // int 3 coerced to float

void main()
{
    float f = toFloat();
    // CHECK: 3
    printf("%d\n", int(f));
}
