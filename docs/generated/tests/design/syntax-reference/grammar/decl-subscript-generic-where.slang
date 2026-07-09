//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=5e6e6830e19c77586abe7afcb70af45ad2c1dc56341e670eb4abae10eeb65319
//META: purpose=`SubscriptDecl` accepts inline generic params and a `WhereClause` between the `'->' Type` and the accessor block.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `__subscript` is routed through the generic machinery:
// a subscript with its own type parameter and a trailing where-clause
// (`__subscript<T>(T key) -> int where T : IIdx`). Indexing with a
// conforming key type returns the element selected via the constraint's
// `toIndex()`, proving the whole header parsed.

//TEST:INTERPRET(filecheck=CHECK):

interface IIdx
{
    int toIndex();
}

struct Key : IIdx
{
    int v;
    int toIndex() { return v; }
}

struct Box
{
    int data[3];

    __subscript<T>(T key) -> int where T : IIdx
    {
        get { return data[key.toIndex()]; }
    }
}

void main()
{
    Box b;
    b.data[0] = 10;
    b.data[1] = 20;
    b.data[2] = 30;
    Key k;
    k.v = 1;
    // CHECK: r=20
    printf("r=%d\n", b[k]);
}
