//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=A TupleType with four heterogeneous element types (half, float, double, int) preserves each position's declared type.
//META: intent=expansion
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: TupleType element-type heterogeneity probe. Four
// distinct numeric element types occupy four positions; each
// position read returns the matching declared type.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Tuple<float, double, int, uint> t = makeTuple(1.5f, 2.5lf, 3, 4u);
    // CHECK: t=1,2,3,4
    printf("t=%d,%d,%d,%d\n", int(t._0), int(t._1), t._2, int(t._3));
}
