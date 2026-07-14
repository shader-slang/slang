//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=acec388be6a124a34086d58d95713bb5cd33eeac4b8fcf6534173cfc8ec11143
//META: purpose=A TupleType holds a heterogeneous member-type list; the i-th element is reachable as ._i and has the i-th declared type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies TupleType is Tuple<T1,T2,...> with per-position member
// types reachable as ._0, ._1, ... Mixing int/float/bool across
// positions exercises the per-position type carry; CHECK asserts each
// element reads back at its declared type.

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
