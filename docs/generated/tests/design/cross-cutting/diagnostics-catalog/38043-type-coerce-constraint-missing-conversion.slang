//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=744e4965649072220829a5fd98254549f17401eff3508d48e97c012034c8bb58
//META: purpose=Fires diagnostic E38043 (type-coerce-constraint-missing-conversion) - type coerce constraint not satisfied
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38043
//META: catalog_name=type-coerce-constraint-missing-conversion
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `where T(U)` type-coerce constraint is not satisfiable
// because no conversion exists.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct A {}
struct B {}

void g<T>(T v) where T(B) {}

void main()
{
    A a;
    g<A>(a);
}

//CHECK: E38043
