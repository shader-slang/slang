//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=719124392380f70c62a7eaec5d0be085dfe259bd4bd8b5283c9602a901f256ee
//META: purpose=The checker finds a witness that a type satisfies an interface required by a constraint, so a generic constrained on the interface accepts conforming types.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says slang-check-conformance.cpp finds (or synthesizes)
// a witness that a type satisfies a required interface. Verify
// the user-visible consequence: a generic constrained `T : IFoo`
// accepts a struct that implements IFoo.

//TEST:INTERPRET(filecheck=CHECK):

interface IFoo
{
    int code();
}

struct A : IFoo
{
    int code() { return 42; }
}

int useIface<T : IFoo>(T t) { return t.code(); }

void main()
{
    A a;
    // CHECK: code=42
    printf("code=%d\n", useIface(a));
}
