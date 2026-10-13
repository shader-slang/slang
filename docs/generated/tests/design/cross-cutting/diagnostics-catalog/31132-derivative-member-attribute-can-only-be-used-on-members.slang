//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e1e4289ccf2cf8781d5b7af4ef06072d4eb6dd19fdb6190ebbd2a0e81eae267f
//META: purpose=Fires diagnostic E31132 (derivative-member-attribute-can-only-be-used-on-members) -- DerivativeMember on non-member
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31132
//META: catalog_name=derivative-member-attribute-can-only-be-used-on-members
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[DerivativeMember(x)]
float f;


//CHECK: E31132
