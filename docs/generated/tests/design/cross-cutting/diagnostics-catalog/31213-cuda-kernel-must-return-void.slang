//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=dbbdcdbd50b8d41758446399be6e3e88fb8453b08c9491480435093ead3da448
//META: purpose=Fires diagnostic E31213 (cuda-kernel-must-return-void) -- CUDA kernel return type error
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31213
//META: catalog_name=cuda-kernel-must-return-void
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[CudaKernel]
int main() { return 0; }


//CHECK: E31213
