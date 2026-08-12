//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
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
