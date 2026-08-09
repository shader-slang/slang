//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=57d9f151be60063d08c61244f7768487d8eaebc83717628353b6150f280dd55a
//META: purpose=An explicitly written generic argument for a parameter that also has a default is kept, because only the explicit argument prefix is handed to the constraint solver as fixed input.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the explicitly supplied generic-argument prefix is treated as
// fixed caller input rather than being re-derived. `U` has the default `One`,
// and the call writes `Two` for it; if the default were allowed to overwrite
// the user-written argument the second digit would come back as 1. CHECK
// asserts 22, so both positions kept the argument the caller wrote.

//TEST:INTERPRET(filecheck=CHECK):

interface ITag
{
    static int tag();
}

struct One : ITag
{
    static int tag() { return 1; }
}

struct Two : ITag
{
    static int tag() { return 2; }
}

int probe<T : ITag, U : ITag = One>()
{
    return T.tag() * 10 + U.tag();
}

void main()
{
    // CHECK: v=22
    printf("v=%d\n", probe<Two, Two>());
}
