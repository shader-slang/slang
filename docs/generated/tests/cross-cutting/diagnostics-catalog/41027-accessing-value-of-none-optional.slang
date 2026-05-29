//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E41027 (accessing-value-of-none-optional) - accessing .value on an Optional that is always none
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41027
//META: catalog_name=accessing-value-of-none-optional
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Reading `.value` from an Optional that is statically `none`
// triggers the none-optional-access diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main() {
    Optional<int> opt = none;
    o[0] = opt.value;
}

// CHECK: E41027
