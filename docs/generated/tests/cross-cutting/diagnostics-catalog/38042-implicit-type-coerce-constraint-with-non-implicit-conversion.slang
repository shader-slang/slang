//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E38042
