//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=02bdaa234074e40f6f48635687cd0c1cf99fd86fc36701fbe4f22573b9384f99
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
