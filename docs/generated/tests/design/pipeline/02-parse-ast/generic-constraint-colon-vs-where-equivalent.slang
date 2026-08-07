//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=2adc733a81916b42380544b008460df5b366206d39b999775293a73fe57f1756
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
