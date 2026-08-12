//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=57d9f151be60063d08c61244f7768487d8eaebc83717628353b6150f280dd55a
//META: purpose=An associated-type constraint written as a standalone `__constraint A : IBar` inside the interface is recorded identically to the other two spellings.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third spelling: the constraint is written as a sibling of the
// associated type rather than attached to it, which is exactly the uniform
// representation the checker builds for all three. If this spelling were
// nested under `Item` instead, the generic body could not reach `bar()`.
// CHECK asserts the conformer's value came back.

//TEST:INTERPRET(filecheck=CHECK):

interface IBar
{
    int bar();
}

interface IHolder
{
    associatedtype Item;
    __constraint Item : IBar;
    Item get();
}

struct Coin : IBar
{
    int bar() { return 5; }
}

struct Purse : IHolder
{
    typealias Item = Coin;
    Coin get() { Coin c; return c; }
}

int use<T : IHolder>(T holder)
{
    return holder.get().bar();
}

void main()
{
    Purse p;
    // CHECK: v=5
    printf("v=%d\n", use(p));
}
