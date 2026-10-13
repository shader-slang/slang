//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=98118594ff9e816dd24b106e430ffd46bbebdbba762c5e4b1de6fd44e5983c13
//META: purpose=Fires diagnostic E38042 (implicit-type-coerce-constraint-with-non-implicit-conversion) - implicit conversion constraint not satisfied
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38042
//META: catalog_name=implicit-type-coerce-constraint-with-non-implicit-conversion
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An `implicit` type-coerce constraint requires the conversion
// to be implicit; an explicit-only converter violates this and
// triggers E38042.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct B {}

struct A
{
    __init(B b) {}
}

void g<T>(T v) where A(T) implicit {}

void main()
{
    B b;
    g<B>(b);
}

//CHECK: E38042
