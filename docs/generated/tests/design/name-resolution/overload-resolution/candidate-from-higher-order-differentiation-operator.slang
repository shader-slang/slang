//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-where-candidates-come-from
//META: doc_section_digest=09e8a26796107f4ea870ece7161025de2c11ac3fe6f559cd6a8ba18ea88a706d
//META: purpose=AddHigherOrderOverloadCandidates contributes candidates for operators such as __fwd_diff that wrap a callee, so the wrapped function resolves and is applied.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the higher-order candidate family. The callee is not a plain
// decl-ref but `__fwd_diff(sq)`, whose candidate is built by the higher-order
// helper from the wrapped function. CHECK asserts the derivative value 6, which
// is only produced if the wrapping candidate resolved and the call was built.

//TEST:INTERPRET(filecheck=CHECK):

[Differentiable]
float sq(float x) { return x * x; }

void main()
{
    var r = __fwd_diff(sq)(diffPair(3.0, 1.0));
    // CHECK: d=6.0
    printf("d=%.1f\n", r.d);
}
