//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: 30103
