//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=f9c7becda59ea2cafbc68bded1f9fb4c90f4bada02e2725c3fe5655439f0c486
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
