//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30102 (decl-not-allowed) - declaration not allowed here
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30102
//META: catalog_name=decl-not-allowed
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A namespace declaration nested inside a function body is not allowed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    namespace N { int x; }
}

// CHECK: E30102
