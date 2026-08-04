//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E36118 (sub-type-has-subset-of-abstract-atoms-to-super-type) - subtype missing target/stage support
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36118
//META: catalog_name=sub-type-has-subset-of-abstract-atoms-to-super-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A subtype declares fewer target capabilities than the
// supertype, triggering E36118.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry main -profile sm_6_0

[require(hlsl, glsl)]
interface IFoo
{
    void f();
}

[require(hlsl)]
struct S : IFoo
{
    void f() {}
}

[numthreads(1,1,1)]
void main() {}

// CHECK: E36118
