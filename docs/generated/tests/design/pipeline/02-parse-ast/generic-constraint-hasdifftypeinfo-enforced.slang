//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=eb0fd6263e6c05fcda56c26f5dbe0c80c738c9165b8bfd51abc3dc4235097f39
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
