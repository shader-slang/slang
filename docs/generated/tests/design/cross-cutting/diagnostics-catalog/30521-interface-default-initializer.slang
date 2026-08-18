//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c5ba9146e047804cc041c3bc4e0edb99f87aaf0313dc49771caae632dbe26e7a
//META: purpose=Fires diagnostic E30521 (interface-default-initializer) -- initializing an interface variable with defaults is deprecated and may cause unexpected behavior
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30521
//META: catalog_name=interface-default-initializer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo {}
[shader("compute")][numthreads(1,1,1)]
void main() {
    IFoo x = {};
}


//CHECK: E30521
