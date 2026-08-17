//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=eb0fd6263e6c05fcda56c26f5dbe0c80c738c9165b8bfd51abc3dc4235097f39
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
