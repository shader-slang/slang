//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=2adc733a81916b42380544b008460df5b366206d39b999775293a73fe57f1756
//META: purpose=The `where __hasDiffTypeInfo(T)` spelling parses into its own constraint decl, which the checker enforces at specialization (E30440 when unsatisfied).
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `__hasDiffTypeInfo(T)` where-clause spelling produces
// a real, enforced constraint: specializing with a plain struct that
// carries no differentiable-type info fails with the
// constraint-not-satisfied diagnostic naming the constraint. That the
// error renders `__hasDiffTypeInfo(T)` proves the spelling parsed into
// a dedicated constraint decl rather than being dropped.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Plain
{
    int v;
}

T passDiff<T>(T x) where __hasDiffTypeInfo(T)
{
    return x;
}

void main()
{
    Plain p;
    passDiff(p);
//CHECK: E30440
//CHECK: could not satisfy the generic constraint '__hasDiffTypeInfo(T)'
}
