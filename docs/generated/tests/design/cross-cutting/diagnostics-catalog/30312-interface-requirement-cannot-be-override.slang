//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b8d32e907cf217b0d8220a8aba833e776cdb1937ddfef52d2fe51eccb46649c2
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

//CHECK: 30312
