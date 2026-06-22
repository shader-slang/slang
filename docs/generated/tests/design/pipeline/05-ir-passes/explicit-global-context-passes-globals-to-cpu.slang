//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=06fba085c49c1b3a2b0f9fcc4941f5b7e0c382078660bd5b19c43e0aa5b68350
//META: purpose=The explicit-global-context pass threads the global parameter context through the C++ emit; `GlobalParams_0` is passed by pointer rather than read from a literal global.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Explicit global context" row of the layout-and-binding
// category (`slang-ir-explicit-global-context.cpp`) threads global
// context through calls. On the C++ target the emit shows a
// `GlobalParams_0*` parameter / pointer dereference for the
// global-uniform reads.

//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// CPP: GlobalParams
// CPP: SLANG_PRELUDE_EXPORT
