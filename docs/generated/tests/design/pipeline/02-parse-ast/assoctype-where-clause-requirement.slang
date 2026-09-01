//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#interface-associated-type-and-__constraint-requirements
//META: doc_section_digest=87eaa476e05e936a2bb0b7cd0ad066839d24b61d75d234809ccf6e876c8763e3
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
