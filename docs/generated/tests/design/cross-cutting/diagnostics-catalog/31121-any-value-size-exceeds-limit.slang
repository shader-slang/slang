//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=bb085ef48d95b0dde56bc879fb9821893b1d041bb13fa996e61e40cfdda4e1b0
//META: purpose=Fires diagnostic E31121 (any-value-size-exceeds-limit) -- 'anyValueSize' exceeds limit
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31121
//META: catalog_name=any-value-size-exceeds-limit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[anyValueSize(99999)]
interface I {}


//CHECK: E31121
