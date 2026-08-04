//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=f9c7becda59ea2cafbc68bded1f9fb4c90f4bada02e2725c3fe5655439f0c486
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
