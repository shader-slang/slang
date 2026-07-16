//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=49a7ad484e2dfba8ca655298b70ff54f8a19782584f47ca1a503b190c156a5cf
//META: purpose=`InterfaceConstraintDecl` parses as `'__constraint' Type '==' Type ';'` inside an interface body (equality constraint).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the equality form of the interface-level `__constraint`
// requirement: a derived interface pins an inherited associated type
// to `int`. A generic over the derived interface then does integer
// arithmetic on `t.get()` directly, which only type-checks if the
// equality constraint refined the associated type.

//TEST:INTERPRET(filecheck=CHECK):

interface IBase
{
    associatedtype D;
    D get();
}

interface IDerived : IBase
{
    __constraint D == int;
}

struct S : IDerived
{
    typealias D = int;
    int get() { return 40; }
}

int probe<T : IDerived>(T t)
{
    return t.get() + 2;
}

void main()
{
    S s;
    // CHECK: r=42
    printf("r=%d\n", probe(s));
}
