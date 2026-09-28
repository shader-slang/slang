//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=d7caae452a61c310382c555cefddbf2b170195ba7546beedf735d5f02266c950
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

//CHECK: 30030
