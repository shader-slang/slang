//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=d3749fed9fbc71b7128e3f169c932422f533a751a10b2b556022f040c1cb72fb
//META: purpose=A TupleType holds a heterogeneous member-type list; the i-th element is reachable as ._i and has the i-th declared type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `TupleType` is the `DeclRefType` for `Tuple<T1, T2, ...>`. The
// canonical surface is `makeTuple(a, b, c)`, and the i-th member is
// reachable as `._i` with the corresponding declared type. Mixing
// types across positions exercises the per-position type carry.

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
