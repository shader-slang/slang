//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#accessordecl-family
//META: doc_section_digest=7ab459ecbfa076cb34ec94ae11dca1b7bd65f74d607addf906138dac14f78530
//META: purpose=A subscript requirement written with an empty body is treated like `{ get; }` and is satisfied by a conformer that supplies an explicit getter.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the subscript half of the implicit-getter rule. The interface's
// `__subscript` requirement has an empty body; the conforming struct supplies
// an explicit `get`. Indexing through a constrained generic must reach that
// getter, and the CHECK asserts its result.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasS { __subscript(int i) -> int { } }

struct S : IHasS
{
    int v;
    __subscript(int i) -> int { get { return v * i; } }
}

int readS<T : IHasS>(T t) { return t[3]; }

void main()
{
    S s; s.v = 4;
    //CHECK: 12
    printf("%d\n", readS(s));
}
