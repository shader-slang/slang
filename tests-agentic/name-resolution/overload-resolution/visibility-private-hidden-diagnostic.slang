//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=c71eed362b7090c94c5447396e3ebd7ff5d3d15480a793a8f05cdaa4015e935b
//META: purpose=TryCheckOverloadCandidateVisibility rejects a candidate whose declaration is not visible from the source scope; a private member accessed outside its host emits decl-is-not-visible (30600).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "TryCheckOverloadCandidateVisibility ... delegates to
// isDeclVisibleFromScope. In ForReal mode emits diagnostic
// decl-is-not-visible (30600)". A private member is invisible from
// outside its host type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S
{
    private int secret() { return 42; }
}

void main()
{
    S s;
    int v = s.secret();
//CHECK:      ^^^^^^ declaration not accessible
// CHECK: E30600
}
