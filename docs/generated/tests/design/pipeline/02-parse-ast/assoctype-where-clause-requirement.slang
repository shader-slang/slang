//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#interface-associated-type-and-__constraint-requirements
//META: doc_section_digest=9506ab20aaa39e5ee0d868d0a15106a005c8e09b8482625b171772602af36266
//META: purpose=An `associatedtype A where A : IBar` where-clause constraint is relocated to the enclosing interface and behaves identically to the inheritance-clause spelling.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the where-clause spelling of an associated-type constraint
// is relocated to the enclosing interface exactly like the
// inheritance-clause form: generic code bounded by the outer
// interface can call the constrained interface's method on the
// associated-type value.

//TEST:INTERPRET(filecheck=CHECK):

interface ICounter
{
    int count();
}

interface IHolder
{
    associatedtype Item where Item : ICounter;
    Item getItem();
}

struct Coin : ICounter
{
    int n;
    int count() { return n; }
}

struct Purse : IHolder
{
    typealias Item = Coin;
    Coin c;
    Coin getItem() { return c; }
}

int countItem<T : IHolder>(T holder)
{
    return holder.getItem().count();
}

void main()
{
    Purse p;
    p.c.n = 13;
    // CHECK: count=13
    printf("count=%d\n", countItem(p));
}
