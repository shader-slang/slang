//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=49a7ad484e2dfba8ca655298b70ff54f8a19782584f47ca1a503b190c156a5cf
//META: purpose=`AssocTypeDecl` accepts a trailing `WhereClause` before `';'`, equivalent to the `':' TypeList` bound.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the where-clause spelling of an associated-type constraint:
// `associatedtype Elem where Elem : IVal;`. A generic constrained to the
// interface calls `val()` through the associated type, which only
// type-checks if the where-clause bound was recorded as a constraint.

//TEST:INTERPRET(filecheck=CHECK):

interface IVal
{
    int val();
}

interface IHolder
{
    associatedtype Elem where Elem : IVal;
    Elem get();
}

struct Item : IVal
{
    int v;
    int val() { return v; }
}

struct Holder : IHolder
{
    typealias Elem = Item;
    Item get()
    {
        Item i;
        i.v = 21;
        return i;
    }
}

int probe<T : IHolder>(T h)
{
    return h.get().val() * 2;
}

void main()
{
    Holder h;
    // CHECK: r=42
    printf("r=%d\n", probe(h));
}
