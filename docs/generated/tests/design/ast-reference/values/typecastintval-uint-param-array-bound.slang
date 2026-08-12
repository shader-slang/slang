//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=7dc220c4c4dd5c2b21296a00da8c0b8bb7caf5c69af13f6b032f0ae0e4047ecd
//META: purpose=An integer cast to a different integer type is itself a compile-time integer, so a uint generic value parameter cast to int is usable as an array bound and folds after substitution.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a cast between integer types survives as a compile-time
// value rather than forcing the bound to become non-constant. The
// parameter type `int[int(N)]` is built over a `uint` generic value
// parameter; the call `sumCast<3>(aa)` with an `int[3]` argument only
// type-checks if the cast value substituted and folded to 3, and CHECK
// asserts the resulting sum.

//TEST:INTERPRET(filecheck=CHECK):

int sumCast<let N : uint>(int a[int(N)])
{
    int total = 0;
    for (int i = 0; i < int(N); ++i) { total += a[i]; }
    return total;
}

void main()
{
    int aa[3] = { 1, 2, 3 };
    // CHECK: cast=6
    printf("cast=%d\n", sumCast<3>(aa));
}
