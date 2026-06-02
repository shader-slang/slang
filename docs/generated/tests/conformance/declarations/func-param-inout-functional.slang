//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#parameters
//META: doc_section_digest=00c2961b7ec63f08cdde68ad692eff760da88176385d08086919fbf37306beee
//META: purpose=An `inout` parameter uses copy-in-copy-out semantics: caller's value is copied in, callee may modify it, and the final value is copied back.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "`in out` or `inout` indicates pass-by-value-result (copy-in
// and copy-out) semantics."

//TEST:INTERPRET(filecheck=CHECK):

void accumulate(inout int sum, int val) { sum += val; }

void main()
{
    int total = 0;
    accumulate(total, 5);
    accumulate(total, 3);
    // CHECK: 8
    printf("%d\n", total);
}
