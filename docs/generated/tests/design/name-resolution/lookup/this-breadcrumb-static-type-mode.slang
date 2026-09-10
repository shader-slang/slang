//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=32b5d6cdc50501bef4e7ef2a5cc9d61e93145ad0a37a1049859831fc766c24f1
//META: purpose=An effectively-static member function leaves only the This type rather than a this value, so an unqualified name in its body resolves to a static member of the enclosing type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the static arm of the this-parameter-mode update. A static
// method has no receiver, so the breadcrumb the scope walk records can
// only name the enclosing type; the unqualified read of `factor` must
// therefore bind to the static field. CHECK asserts 5 * 4, which is
// unreachable if `factor` failed to resolve without a receiver.

//TEST:INTERPRET(filecheck=CHECK):

struct U
{
    static const int factor = 4;
    static int scale(int x) { return x * factor; }
}

void main()
{
    // CHECK: r=20
    printf("r=%d\n", U::scale(5));
}
