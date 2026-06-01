//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30317 (non-method-interface-requirement-cannot-have-body) - interface requirement has body
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30317
//META: catalog_name=non-method-interface-requirement-cannot-have-body
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A non-method interface requirement (subscript getter) with a body is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IHolder
{
    __subscript(int i) -> int { get { return 0; } }
}

void main() {}

// CHECK: 30317
