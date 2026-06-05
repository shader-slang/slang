//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=Passing a type that does not satisfy the declared constraint is a compile-time error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that passing a type that does not conform to the declared constraint is rejected.
// `usesFoo<T : IFoo>` requires IFoo; passing `NotFoo` (no conformance) should trigger E39999,
// confirming that constraint checking fires before specialization.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo { int val(); }

int usesFoo<T : IFoo>(T x) { return x.val(); }

struct NotFoo { int x; }

void test()
{
    NotFoo nf;
    nf.x = 1;
    int r = usesFoo(nf);
//CHECK: E39999
}
