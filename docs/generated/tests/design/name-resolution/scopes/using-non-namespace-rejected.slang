//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:00Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=5c32316c757a32af0aa6a61ee5f325f4830e8f73bc176d8904fdb4ddc1d8d758
//META: purpose=When a `using` argument does not resolve to a namespace-like entity, no sibling scope is added and the checker diagnoses expected-a-namespace (E30061).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the failure path of using-decl scope wiring: naming a plain
// global variable in `using` cannot yield a namespace-like target, so no
// sibling is spliced and the E30061 expected-a-namespace diagnostic must
// fire at the argument.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int notANamespace = 3;

namespace N
{
    using notANamespace;
//CHECK:  ^^^^^^^^^^^^^ expected a namespace
//CHECK:  ^^^^^^^^^^^^^ expected a namespace, got a 'int'
// CHECK: E30061
}
