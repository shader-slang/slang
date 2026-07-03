//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=de0d5bd1ea808c72b5b22ad5fbfc389ef05a460b63cf6e8aa1c1f87a20e14622
//META: purpose=Overload resolution rejects a call passing too many arguments with the "too many arguments to call" diagnostic, counterpart of "not enough arguments".
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary of arity coverage: companion to no-applicable-overload's
// "not enough arguments" case — verify the opposite, too-many-args,
// also fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void foo() {}
/*CHECK:
     ^^^ candidate: func foo() -> void
*/

void main() { foo(1, 2); }
//CHECK:         ^ too many arguments to call
//CHECK:         ^ too many arguments to call (got 2, expected 0)
