//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#accessordecl-family
//META: doc_section_digest=8cebf6056445131d843ba65809820ad0e5294659556653bcb7502aa0a4f723ef
//META: purpose=A property requirement written with an empty body is treated like `{ get; }` and is satisfied by a conformer that supplies an explicit getter.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an interface property requirement whose body contains no
// explicit accessor keyword still behaves as a get-only requirement. The
// conforming struct supplies an explicit `get`; a constrained generic reads
// the property and the CHECK asserts the getter's result, which is only
// reachable if the empty requirement body materialized a getter to satisfy.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasP { property int p { } }

struct S : IHasP
{
    int v;
    property int p { get { return v + 1; } }
}

int readP<T : IHasP>(T t) { return t.p; }

void main()
{
    S s; s.v = 4;
    //CHECK: 5
    printf("%d\n", readP(s));
}
