//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=eb0fd6263e6c05fcda56c26f5dbe0c80c738c9165b8bfd51abc3dc4235097f39
//META: purpose=The inheritance-clause form `<T : IFoo>` and the `where T : IFoo` form both parse into constraint decls that grant the generic body the same interface capability.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two documented constraint parse paths — the
// inheritance-clause spelling on the parameter and the `where` clause
// after the parameter list — produce equivalent constraints. The same
// interface method is called through both spellings; both calls only
// type-check if each parse path attached a working type bound.

//TEST:INTERPRET(filecheck=CHECK):

interface IScored
{
    int score();
}

struct Player : IScored
{
    int s;
    int score() { return s; }
}

int viaColon<T : IScored>(T v)
{
    return v.score();
}

int viaWhere<T>(T v) where T : IScored
{
    return v.score();
}

void main()
{
    Player p;
    p.s = 21;
    // CHECK: colon=21 where=21
    printf("colon=%d where=%d\n", viaColon(p), viaWhere(p));
}
