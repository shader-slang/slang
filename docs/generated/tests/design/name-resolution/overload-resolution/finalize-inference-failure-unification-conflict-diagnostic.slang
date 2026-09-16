//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#finalize-phase-completeoverloadcandidate
//META: doc_section_digest=90d8366fb34b230a0bf9d002a41000fb52e4433f1d99750051158a6784953ce9
//META: purpose=When generic inference deduces two conflicting bindings for one parameter, the GenericParamUnificationConflict arm reports both conflicting deductions.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unification-conflict arm of the inference-failure switch. Both
// parameters of `f` are typed T, and the two arguments deduce incompatible
// bindings. CHECK pins the message that names both deduced types, which is the
// evidence that the focused arm fired rather than the blanket fallback.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Custom { int v; }

int f<T>(T a, T b) { return 1; }

void main()
{
    Custom c;
    int r = f(1, c);
//CHECK:     ^ cannot deduce generic argument for 'T': conflicting requirements 'int' and 'Custom'
    printf("%d\n", r);
}
