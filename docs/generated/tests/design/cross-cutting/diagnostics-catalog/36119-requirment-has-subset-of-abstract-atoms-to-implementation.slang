//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E36119 (requirment-has-subset-of-abstract-atoms-to-implementation) - requirement missing target/stage support
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36119
//META: catalog_name=requirment-has-subset-of-abstract-atoms-to-implementation
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An interface requirement declared with fewer target
// capabilities than its implementer triggers E36119.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry main -profile sm_6_0

interface IFoo
{
    [require(hlsl)]
    void f();
}

struct S : IFoo
{
    [require(hlsl, glsl)]
    void f() {}
}

[numthreads(1,1,1)]
void main() {}

// CHECK: E36119
