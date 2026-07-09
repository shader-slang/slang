//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: E38043
