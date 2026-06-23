//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: E41403
