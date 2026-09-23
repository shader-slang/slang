//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=d9357662d809198b672de964a655fbd41120a6b17b51411c05a2be64457de64f
//META: purpose=Expression checking rejects a literal passed to an `out` parameter because the argument must be an l-value (E30047).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that lvalue-ness is enforced at argument-passing sites too, not
// just at `=`. An `out` parameter needs storage to write back into, and a
// literal has none; CHECK pins E30047 on the offending argument.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void take(out int o)
{
    o = 1;
}

void main()
{
    take(3);
//CHECK: E30047
}
