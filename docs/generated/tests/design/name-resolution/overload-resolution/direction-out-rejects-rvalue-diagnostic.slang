//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=c71eed362b7090c94c5447396e3ebd7ff5d3d15480a793a8f05cdaa4015e935b
//META: purpose=TryCheckOverloadCandidateDirections enforces parameter directions; passing a literal r-value to an `out` parameter fails the direction step (argument-must-be-l-value).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "TryCheckOverloadCandidateDirections ... enforces parameter
// directions (in, out, inout, ref). An out parameter must receive
// an l-value; passing a literal to inout fails here." Here we pass
// a literal to an `out` parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void produce(out int dst) { dst = 5; }

void main()
{
    produce(7);
//CHECK:    ^ argument must be l-value
// CHECK: E30047
}
