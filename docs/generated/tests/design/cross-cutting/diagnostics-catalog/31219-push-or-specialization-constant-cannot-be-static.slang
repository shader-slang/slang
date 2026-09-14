//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a7bc419a78aaa8aad78f72a1a3efb85da2a381a5bc1379c4c81d9446b87d0627
//META: purpose=Fires diagnostic E31219 (push-or-specialization-constant-cannot-be-static) -- push/specialization constant storage class error
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31219
//META: catalog_name=push-or-specialization-constant-cannot-be-static
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[SpecializationConstant] static const int x = 5;
[shader("compute")][numthreads(1,1,1)]
void main() {}


//CHECK: E31219
