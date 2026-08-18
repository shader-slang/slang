//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=d4ba568f0b56a5f4a2d2211d82ebca0a2bde37d1a35b94f5839525a04f5ba0f6
//META: purpose=Fires diagnostic E30103 (expect-type-pack-after-each) - expected type pack after 'each'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30103
//META: catalog_name=expect-type-pack-after-each
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `each` applied to a non-pack scalar parameter is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f<T>()
{
    int x = expand sizeof(each T);
}

void main() {}

//CHECK: 30103
