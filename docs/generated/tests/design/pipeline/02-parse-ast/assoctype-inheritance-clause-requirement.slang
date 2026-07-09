//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#interface-associated-type-and-__constraint-requirements
//META: doc_section_digest=9506ab20aaa39e5ee0d868d0a15106a005c8e09b8482625b171772602af36266
//META: purpose=An `associatedtype A : IBar` inheritance-clause constraint becomes an interface-level requirement visible to generic code constrained by the interface.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the inheritance-clause spelling of an associated-type
// constraint acts as an interface-level requirement. Generic code
// bounded only by the outer interface calls `count()` on a value of
// the associated type — that call type-checks only if the constraint
// was hoisted to the interface as a sibling requirement.

//TEST:INTERPRET(filecheck=CHECK):

interface ICounter
{
    int count();
}

interface IHolder
{
    associatedtype Item : ICounter;
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
    p.c.n = 8;
    // CHECK: count=8
    printf("count=%d\n", countItem(p));
}
