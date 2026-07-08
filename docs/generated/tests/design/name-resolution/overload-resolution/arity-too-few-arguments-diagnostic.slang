//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=f9c7becda59ea2cafbc68bded1f9fb4c90f4bada02e2725c3fe5655439f0c486
//META: purpose=TryCheckOverloadCandidateArity rejects a call with fewer arguments than required; in ForReal the resolver emits not-enough-arguments-to-call (code 39999).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "TryCheckOverloadCandidateArity ... verifies the call has between
// required and allowed arguments. Failures in ForReal mode emit
// not-enough-arguments-for-call / too-many-arguments-for-call".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int add(int a, int b) { return a + b; }

void main()
{
    int v = add(7);
//CHECK:       ^ not enough arguments to call
//CHECK: E39999
}
