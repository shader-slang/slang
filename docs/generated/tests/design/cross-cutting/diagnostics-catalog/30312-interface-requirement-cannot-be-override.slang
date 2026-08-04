//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30312 (interface-requirement-cannot-be-override) - interface requirement cannot override
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30312
//META: catalog_name=interface-requirement-cannot-be-override
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using `override` on an interface requirement is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IBase { int get(); }

interface IDerived : IBase
{
    override int get();
}

void main() {}

// CHECK: 30312
