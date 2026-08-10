//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=51cb62c0b32d5853269f2948b06d5fe885de6c3a839dfdb3e56f3ab895330c38
//META: purpose=A requirement that the interface supplies a default body for is satisfied by a witness the checker synthesizes, so the conformer need not declare it.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "default conformance witnesses synthesized" claim. `S`
// implements only `base()`; the `twice()` requirement is satisfied by a
// witness pointing at the interface's default body, and that body calls back
// into the conformer's own `base()`. CHECK asserts 12 = 6 * 2, which proves
// the synthesized witness ran and dispatched dynamically.

//TEST:INTERPRET(filecheck=CHECK):

interface IFoo
{
    int base();
    int twice()
    {
        return base() * 2;
    }
}

struct S : IFoo
{
    int base() { return 6; }
}

int call<T : IFoo>(T v)
{
    return v.twice();
}

void main()
{
    S s;
    // CHECK: v=12
    printf("v=%d\n", call(s));
}
