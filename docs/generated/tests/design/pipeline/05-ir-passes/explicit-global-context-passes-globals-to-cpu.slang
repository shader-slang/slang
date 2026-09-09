//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=7c4335ef62cc708fc063656220fb481b4e3fdbe6592973946fe16a93c217487e
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
