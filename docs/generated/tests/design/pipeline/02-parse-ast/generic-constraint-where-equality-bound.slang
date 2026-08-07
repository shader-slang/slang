//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=2adc733a81916b42380544b008460df5b366206d39b999775293a73fe57f1756
//META: purpose=The `where` clause accepts a type-equality bound (`T.Elem == int`), which parses into a GenericTypeConstraintDecl the checker enforces.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the type-equality spelling of a `where` bound. The generic
// constrains an associated type to equal `int`, and the body adds 1 to
// the associated-type value — legal only if the equality constraint
// parsed and lets the checker treat `T.Elem` as `int`.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasElem
{
    associatedtype Elem;
    Elem get();
}

struct IntBox : IHasElem
{
    typealias Elem = int;
    int v;
    int get() { return v; }
}

int plusOne<T : IHasElem>(T box) where T.Elem == int
{
    return box.get() + 1;
}

void main()
{
    IntBox b;
    b.v = 41;
    // CHECK: eq=42
    printf("eq=%d\n", plusOne(b));
}
