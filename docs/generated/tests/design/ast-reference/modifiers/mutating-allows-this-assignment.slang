//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#mutability--autodiff-annotations
//META: doc_section_digest=a162d3ea93755dfa0e856855015bcc93b11eb24c29ad4e2f7cda299e7c6be3b7
//META: purpose=`[mutating]` on a method allows assigning to fields through `this`; the mutation is visible after the call.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `[mutating]` does two things at once: it lets the body write a
// field of `this`, and it makes the receiver an l-value at the call site so the
// write is not lost on a copy. `bump()` is called twice on a struct initialised
// to 10 and CHECK asserts 12, which requires both calls' writes to have stuck.

//TEST:INTERPRET(filecheck=CHECK):

struct Counter
{
    int n;

    [mutating]
    void bump()
    {
        n = n + 1;
    }
}

void main()
{
    Counter c;
    c.n = 10;
    c.bump();
    c.bump();
    //CHECK: n=12
    printf("n=%d\n", c.n);
}
