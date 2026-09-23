//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=cd298b100ea879e321ba8c00dd3b0c3a1c1a97487df96a2f26509b127a3f0661
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

//CHECK: E30102
