//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ad18b2b11395d952d3621d0c36d19cf197298d0a5042f2d0fc21ee384cabe7b0
//META: purpose=Fires diagnostic E20020 (operator-name-on-non-function) - operator name used to declare a non-function
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20020
//META: catalog_name=operator-name-on-non-function
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An `operator` name may only introduce a function. Declaring it as a variable
// is rejected at parse time; the CHECK pins that code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
int operator+;
void main() {}

//CHECK: E20020
