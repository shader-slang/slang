//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=c71eed362b7090c94c5447396e3ebd7ff5d3d15480a793a8f05cdaa4015e935b
//META: purpose=TryCheckOverloadCandidateArity rejects a call with more arguments than the allowed max; in ForReal the resolver emits too-many-arguments-to-call (code 39999).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "Failures in ForReal mode emit not-enough-arguments-for-call /
// too-many-arguments-for-call". The single-overload candidate
// reaches the finalize phase and emits the arity diagnostic there.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int neg(int a) { return -a; }

void main()
{
    int v = neg(1, 2, 3);
//CHECK:       ^ too many arguments to call
}
