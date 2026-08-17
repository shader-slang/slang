//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
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
