//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#interface-associated-type-and-__constraint-requirements
//META: doc_section_digest=87eaa476e05e936a2bb0b7cd0ad066839d24b61d75d234809ccf6e876c8763e3
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
