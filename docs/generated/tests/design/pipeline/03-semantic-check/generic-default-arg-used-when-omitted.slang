//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=57d9f151be60063d08c61244f7768487d8eaebc83717628353b6150f280dd55a
//META: purpose=A defaulted generic parameter that the caller omits is filled in by the constraint solver's fixpoint, so the default type is used.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the complementary path of the same claim: with no explicit
// argument in that position, the parameter's default has to be routed through
// the solver alongside the inferred and witness arguments. CHECK asserts 21,
// which is `Two` for the written argument and the `One` default for the
// omitted one.

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
    // CHECK: v=21
    printf("v=%d\n", probe<Two>());
}
