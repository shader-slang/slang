//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Expression checking rejects constructor-style initialization of a class without `new` (E30066): classes must be initialized with `new`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Companion negative to new-on-non-class: a class declared as
// `class C { ... }` cannot be initialized with constructor syntax
// `C()` and must use `new C()`. The diagnostic is E30066.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

class C { int x; __init() { x = 0; } }

void main()
{
    C c = C();
//CHECK:   ^ class can only be initialized by `new`
//CHECK:   ^ a class can only be initialized by a `new` clause
// CHECK: E30066
}
