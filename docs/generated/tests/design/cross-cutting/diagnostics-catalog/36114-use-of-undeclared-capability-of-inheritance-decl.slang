//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E36114 (use-of-undeclared-capability-of-inheritance-decl) - capability incompatible with supertype
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36114
//META: catalog_name=use-of-undeclared-capability-of-inheritance-decl
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A subtype declared with fewer capabilities than the supertype
// triggers E36114.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry main -profile sm_6_0

[require(hlsl)]
interface IFoo
{
    void f();
}

[require(glsl)]
struct S : IFoo
{
    void f() {}
}

[numthreads(1,1,1)]
void main() {}

// CHECK: E36114
