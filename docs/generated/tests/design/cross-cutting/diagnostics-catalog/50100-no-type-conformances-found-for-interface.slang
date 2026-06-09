//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E50100 (no-type-conformances-found-for-interface) - no type conformances found
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=50100
//META: catalog_name=no-type-conformances-found-for-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Dynamic dispatch on an interface type requires at least one
// conforming type to be visible in the linkage. With no
// conformances declared, the dispatch site errors.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

interface IFoo { int get(); }

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main() {
    IFoo p;
    o[0] = p.get();
}

// CHECK: E50100
