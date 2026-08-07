//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=49a7ad484e2dfba8ca655298b70ff54f8a19782584f47ca1a503b190c156a5cf
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
