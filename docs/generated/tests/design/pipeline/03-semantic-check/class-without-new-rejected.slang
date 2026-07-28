//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
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
