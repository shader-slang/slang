//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ccdf463ce856ec0755fe81815c29dc8ef033d24fd949afd0c068d9ed167a1b78
//META: purpose=Fires diagnostic E41403 (invalid-atomic-destination-pointer) - invalid atomic destination
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41403
//META: catalog_name=invalid-atomic-destination-pointer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// InterlockedAdd targeting a private local (neither groupshared nor
// a device buffer) is rejected by the atomic-pointer check.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main() {
    int local = 0;
    InterlockedAdd(local, 1);
    o[0] = local;
}

//CHECK: E41403
