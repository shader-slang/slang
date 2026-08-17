//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#requirementdecl-style-nodes-inside-interfacedecl
//META: doc_section_digest=61b41d7c34c3d57c8ab25ebc17ad4847ac05d606da5acd0ee3a97221870d03b4
//META: purpose=Interface requirements are not a distinct declaration class: a property and a subscript written in an interface body are requirements just like a method.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that requirement-ness comes from position, not from a special
// declaration kind. The interface body holds a property, a subscript and a
// method; a constrained generic uses all three on its parameter, and the CHECK
// asserts a value that combines them, so each had to be satisfied by the
// conformer.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasAll
{
    property int p { get; }
    __subscript(int i) -> int { get; }
    int m();
}

struct S : IHasAll
{
    int v;
    property int p { get { return v + 1; } }
    __subscript(int i) -> int { get { return v * i; } }
    int m() { return 100; }
}

int useAll<T : IHasAll>(T t) { return t.p + t[3] + t.m(); }

void main()
{
    S s; s.v = 4;
    //CHECK: 117
    printf("%d\n", useAll(s));
}
