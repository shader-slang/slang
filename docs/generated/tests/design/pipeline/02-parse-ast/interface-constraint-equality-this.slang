//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#interface-associated-type-and-__constraint-requirements
//META: doc_section_digest=87eaa476e05e936a2bb0b7cd0ad066839d24b61d75d234809ccf6e876c8763e3
//META: purpose=The `__constraint DataType == This` equality form inside a derived interface parses and lets generic code treat the inherited associated type as the conforming type itself.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's own `__constraint DataType == This` example. The
// derived interface pins the base's associated type to the conformer,
// so a generic bounded by IDerived can call an IDerived method on
// `getData()`'s result — legal only if the equality constraint parsed
// with isEqualityConstraint semantics.

//TEST:INTERPRET(filecheck=CHECK):

interface IBase
{
    associatedtype DataType;
    DataType getData();
}

interface IDerived : IBase
{
    __constraint DataType == This;
    int value();
}

struct Node : IDerived
{
    typealias DataType = Node;
    int v;
    Node getData() { return this; }
    int value() { return v; }
}

int useEquality<T : IDerived>(T x)
{
    // x.getData() has type T.DataType == T, so value() is callable on it.
    return x.getData().value();
}

void main()
{
    Node n;
    n.v = 27;
    // CHECK: value=27
    printf("value=%d\n", useEquality(n));
}
