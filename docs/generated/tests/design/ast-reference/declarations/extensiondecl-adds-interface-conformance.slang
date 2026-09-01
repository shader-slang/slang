//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#extensiondecl
//META: doc_section_digest=ad88a19c43c96571c45e942725f5abaf6372424eba2bc248fca5d077dd824f5c
//META: purpose=An ExtensionDecl can add an inheritance clause to an existing type, making that type usable where the interface is required.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an extension may add a conformance, not just members. The
// struct does not itself declare `: IDouble`; the extension does. Passing the
// struct to an interface-constrained generic can only type-check if the
// extension's inheritance clause was attached to the original type, and the
// CHECK asserts the dispatched result.

//TEST:INTERPRET(filecheck=CHECK):

interface IDouble { int doubled(); }

struct N { int v; }

extension N : IDouble
{
    int doubled() { return v * 2; }
}

int callDoubled<T : IDouble>(T t) { return t.doubled(); }

void main()
{
    N n; n.v = 5;
    //CHECK: 10
    printf("%d\n", callDoubled(n));
}
