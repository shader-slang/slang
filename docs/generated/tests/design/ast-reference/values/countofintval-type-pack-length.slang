//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=7dc220c4c4dd5c2b21296a00da8c0b8bb7caf5c69af13f6b032f0ae0e4047ecd
//META: purpose=A countof applied to a type pack is a compile-time integer that folds to the number of elements the pack was bound to.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the type-argument form of the compile-time `countof` value.
// `countof(T)` inside the generic is symbolic; the call site binds the
// pack to three arguments, so the returned integer must be 3. CHECK
// asserts the printed count, which is the folded value.

//TEST:INTERPRET(filecheck=CHECK):

int packCount<each T>(expand each T args)
{
    return countof(T);
}

void main()
{
    // CHECK: n=3
    printf("n=%d\n", packCount(1, 2, 3));
}
