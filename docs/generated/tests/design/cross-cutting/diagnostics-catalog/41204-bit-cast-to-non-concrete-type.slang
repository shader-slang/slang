//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E41204 (bit-cast-to-non-concrete-type) - cannot bit-cast to existential (non-concrete) type
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41204
//META: catalog_name=bit-cast-to-non-concrete-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `bit_cast` cannot produce an interface-typed (existential) value
// because the layout is not concrete — error fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

interface IFoo { int get(); }
struct A : IFoo { int v; int get() { return v; } }

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID) {
    A a = {int(tid.x)};
    IFoo y = bit_cast<IFoo, A>(a);
    o[0] = y.get();
}

// CHECK: E41204
