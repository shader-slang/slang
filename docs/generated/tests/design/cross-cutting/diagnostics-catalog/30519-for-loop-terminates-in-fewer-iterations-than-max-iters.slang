//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b7017ee850cf8a18e309bd335773f73013aad7b7694cc68be6a964cbb18f8e5e
//META: purpose=Fires diagnostic E30519 (for-loop-terminates-in-fewer-iterations-than-max-iters) -- MaxIters exceeds actual iterations
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30519
//META: catalog_name=for-loop-terminates-in-fewer-iterations-than-max-iters
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[shader("compute")][numthreads(1,1,1)]
void main() {
    [MaxIters(100)]
    for (int i = 0; i < 5; ++i) {}
}


//CHECK: E30519
