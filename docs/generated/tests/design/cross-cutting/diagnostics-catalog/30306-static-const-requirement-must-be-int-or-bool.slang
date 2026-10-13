//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=21ee7be3d07f15fa7b94b8d8de524fa785cfb2afdc4f6f440d923d69c1f9e467
//META: purpose=Fires diagnostic E30306 (static-const-requirement-must-be-int-or-bool) - invalid static const requirement type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30306
//META: catalog_name=static-const-requirement-must-be-int-or-bool
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An interface 'static const' requirement of a non-int/bool type is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IHasFloat
{
    static const float k;
}

void main() {}

//CHECK: 30306
