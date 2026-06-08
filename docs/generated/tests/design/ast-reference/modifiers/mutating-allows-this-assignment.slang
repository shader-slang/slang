//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#mutability-autodiff-annotations
//META: doc_section_digest=017266f7b5d727836de4cd07bb1c2bfcdb0d82e30647d5be27f19fbfcea1a2dc
//META: purpose=`[mutating]` on a method allows assigning to fields through `this`; the mutation is visible after the call.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `MutatingAttribute` marks a method as allowed to write through
// `this`. After the call, the caller observes the modified field.

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
