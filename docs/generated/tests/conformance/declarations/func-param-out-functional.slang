//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#parameters
//META: doc_section_digest=00c2961b7ec63f08cdde68ad692eff760da88176385d08086919fbf37306beee
//META: purpose=An `out` parameter uses copy-out semantics: the callee writes to it and the value is copied to the caller's argument on return.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies copy-out semantics: the callee writes to `out` parameters and values appear in the caller.
// `getValues` assigns 10 and 20 to uninitialized locals via `out`; `10 20`
// confirms both values were copied back to the call site correctly.

//TEST:INTERPRET(filecheck=CHECK):

void getValues(out int a, out int b)
{
    a = 10;
    b = 20;
}

void main()
{
    int x, y;
    getValues(x, y);
    // CHECK: 10 20
    printf("%d %d\n", x, y);
}
