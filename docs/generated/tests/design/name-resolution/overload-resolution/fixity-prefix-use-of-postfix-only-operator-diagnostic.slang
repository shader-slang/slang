//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=211bc03cd9a092841d1719f62552fc549d768b7b9dc4ea9b6c978e0acc8daa2b
//META: purpose=TryCheckOverloadCandidateFixity rejects a postfix-only operator overload used in prefix position, so -x does not bind to a __postfix operator-.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fixity filter. `operator-` for V carries only __postfix, and the
// use site writes the prefix form, so the fixity step drops the one candidate
// that could otherwise accept a V. CHECK pins the resulting no-overload error,
// which is the user-visible consequence of the fixity rejection.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct V { int x; }

__postfix V operator-(V a) { V r; r.x = -a.x; return r; }

void main()
{
    V a;
    a.x = 5;
    V b = -a;
//CHECK:  ^ no overload for '-' applicable to arguments of type (V)
    printf("%d\n", b.x);
}
