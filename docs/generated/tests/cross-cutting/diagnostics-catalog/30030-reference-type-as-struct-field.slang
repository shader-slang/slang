//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30030 (reference-type-as-struct-field) - reference type cannot be used as struct field
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30030
//META: catalog_name=reference-type-as-struct-field
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct field with a reference (`Ref<T>`) type triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S
{
    Ref<int> r;
};

void main() {}

// CHECK: 30030
