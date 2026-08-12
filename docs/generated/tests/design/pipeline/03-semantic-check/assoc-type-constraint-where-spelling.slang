//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=57d9f151be60063d08c61244f7768487d8eaebc83717628353b6150f280dd55a
//META: purpose=An associated-type constraint written as `associatedtype A where A : IBar` is recorded identically to the inheritance-clause spelling.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `where`-clause spelling reaches the same interface-level
// requirement. The program is the shape of the colon-spelling test with only
// the constraint syntax changed, so a difference in recorded representation
// would show up as a failure to type-check `bar()` in the generic body.
// CHECK asserts the conformer's value came back.

//TEST:INTERPRET(filecheck=CHECK):

interface IBar
{
    int bar();
}

interface IHolder
{
    associatedtype Item where Item : IBar;
    Item get();
}

struct Coin : IBar
{
    int bar() { return 4; }
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
    // CHECK: v=4
    printf("v=%d\n", use(p));
}
