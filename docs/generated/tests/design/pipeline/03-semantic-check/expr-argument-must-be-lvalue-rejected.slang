//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=b5f523113b24470cb3eee4f9c8ae76906f41333e955cb7bb7bac14ed1e5d3a05
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
