//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
//META: purpose=`InterfaceConstraintDecl` parses as `'__constraint' Type ':' Type ';'` inside an interface body (subtype constraint).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the subtype form of the interface-level `__constraint`
// requirement: a derived interface constrains an inherited associated
// type to conform to IVal. A generic over the derived interface calls
// `val()` on the associated-type value, reachable only through that
// subtype constraint.

//TEST:INTERPRET(filecheck=CHECK):

interface IVal
{
    int val();
}

interface IBase
{
    associatedtype D;
    D make();
}

interface IDerived : IBase
{
    __constraint D : IVal;
}

struct Item : IVal
{
    int v;
    int val() { return v; }
}

struct S : IDerived
{
    typealias D = Item;
    Item make()
    {
        Item i;
        i.v = 9;
        return i;
    }
}

int probe<T : IDerived>(T t)
{
    return t.make().val();
}

void main()
{
    S s;
    // CHECK: r=9
    printf("r=%d\n", probe(s));
}
