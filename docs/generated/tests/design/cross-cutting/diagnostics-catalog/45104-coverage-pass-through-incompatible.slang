//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a77306016356bc8d9fdfca555712dee919e8096334564819e32f8da800749a2b
//META: purpose=Fires diagnostic E45104 (coverage-pass-through-incompatible) - `-trace-coverage` cannot be combined with `-pass-through`
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=45104
//META: catalog_name=coverage-pass-through-incompatible
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Combining -trace-coverage with -pass-through bypasses the IR
// pipeline that performs the instrumentation, so the option pair
// is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target dxil -entry main -stage compute -trace-coverage -pass-through dxc

[numthreads(1,1,1)]
void main() {}

//CHECK: E45104
