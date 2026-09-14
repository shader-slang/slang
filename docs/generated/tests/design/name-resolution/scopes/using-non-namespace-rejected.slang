//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=b57e81781278bd6752bd817f50c238aae4f56d4fda646e22befd826b34d6cab4
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
