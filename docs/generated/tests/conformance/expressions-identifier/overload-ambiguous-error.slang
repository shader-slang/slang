//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#overloading
//META: doc_section_digest=9484a135a7d65023e550053886ef224a8110c3717ecb4a6f046fb51a7be6e355
//META: purpose=When an overloaded identifier cannot be disambiguated at the call site, the compiler emits an ambiguous-call error (E39999).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two overloads where neither is strictly preferred for the given argument types.
// The call `foo(1, 1)` cannot disambiguate between foo(int,float) and foo(float,int),
// so an ambiguous-call error must be reported at the use site.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry computeMain -stage compute

void foo(int x, float y) {}
void foo(float x, int y) {}

[numthreads(1, 1, 1)]
void computeMain()
{
    foo(1, 1);
    //CHECK: E39999
}
