//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b7f4a3ca2406aa05241e3747b24fa80c7815595080d8702dc1b6395ebdbdcbf8
//META: purpose=Fires diagnostic E41401 (static-assertion-failure-without-message) - static assertion failed
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=41401
//META: catalog_name=static-assertion-failure-without-message
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A static_assert(false, expr) whose message argument is not a
// literal string falls into the without-message diagnostic path.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

String getMsg() { return "fail"; }

[numthreads(1,1,1)]
void main() {
    static_assert(false, getMsg());
}

//CHECK: E41401
