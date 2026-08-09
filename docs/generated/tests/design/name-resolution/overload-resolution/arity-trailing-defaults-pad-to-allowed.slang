//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=211bc03cd9a092841d1719f62552fc549d768b7b9dc4ea9b6c978e0acc8daa2b
//META: purpose=TryCheckOverloadCandidateArity accepts a call with fewer arguments than parameters when trailing defaults pad the count up to allowed.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "trailing defaults can pad up to allowed" half of the arity
// step, at its lower edge: `pad` has one required and two defaulted
// parameters, and the call supplies exactly the required count. CHECK asserts
// the digits contributed by the two defaults are present, which proves the
// candidate survived the arity step and the defaults were filled in.

//TEST:INTERPRET(filecheck=CHECK):

int pad(int a, int b = 5, int c = 7) { return a * 100 + b * 10 + c; }

void main()
{
    int v = pad(1);
    // CHECK: v=157
    printf("v=%d\n", v);
}
