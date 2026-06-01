//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E31213 (cuda-kernel-must-return-void) -- CUDA kernel return type error
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31213
//META: catalog_name=cuda-kernel-must-return-void
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[CudaKernel]
int main() { return 0; }


// CHECK: E31213
