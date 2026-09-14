//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
//META: purpose=Negative: a `let`-bound name is not an l-value, so it cannot be passed to an `inout` parameter even though the call site reads syntactically fine; the checker rejects with the l-value diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pairs with `decl-let-immutable-rejected.slang`, which rejects a
// direct write. This test probes the boundary where the write is
// indirect (through an `inout` parameter modifier from the
// simple-modifiers table) — the immutability check still fires.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void inc(inout int x) { x = x + 1; }

void main()
{
    let v = 5;
    inc(v);
    // CHECK-DAG: error
    // CHECK-DAG: l-value
    // CHECK-DAG: E30047
    printf("%d\n", v);
}
