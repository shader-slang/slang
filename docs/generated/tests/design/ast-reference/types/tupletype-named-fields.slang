//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=A TupleType holds a per-position member-type list; the i-th element is reachable as `._i` and has the i-th declared type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-position member list of the tuple type by mixing
// int/float/bool across the three positions. Each element is read back
// through `._0`/`._1`/`._2` and converted for printing; CHECK asserts
// all three values, which requires each position to have kept its own
// declared type.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Tuple<int, float, bool> t = makeTuple(7, 1.5f, true);
    // CHECK: t._0=7
    printf("t._0=%d\n", t._0);
    // CHECK: t._1=1
    printf("t._1=%d\n", int(t._1));
    // CHECK: t._2=1
    printf("t._2=%d\n", t._2 ? 1 : 0);
}
