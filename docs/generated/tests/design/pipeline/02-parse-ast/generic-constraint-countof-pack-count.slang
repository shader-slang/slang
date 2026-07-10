//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=2adc733a81916b42380544b008460df5b366206d39b999775293a73fe57f1756
//META: purpose=The `where countof(Pack) == IntExpr` spelling parses into a pack-count constraint that a call with the matching pack arity satisfies.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented one-sided pack-count spelling with
// `countof(...)` on the left. The generic requires exactly N pack
// elements; the explicit specialization `<2>` with a two-element pack
// satisfies the constraint and the sum is printed, proving the
// constraint parsed and solved.

//TEST:INTERPRET(filecheck=CHECK):

int sumExactly<let N : int, each T>(expand each T args) where countof(T) == N where T == int
{
    int total = 0;
    expand(total = total + (each args));
    return total;
}

void main()
{
    // CHECK: sum=11
    printf("sum=%d\n", sumExactly<2>(4, 7));
}
