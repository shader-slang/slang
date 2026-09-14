//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#parameter-binding-producing-the-programlayout
//META: doc_section_digest=ba2ce52ee3140a9f243e5fb99b8f2c9379dfa7db6e7567a4e73f5fca0723c7b2
//META: purpose=An explicit HLSL register annotation on a global parameter is likewise reserved before automatic allocation of the remaining globals.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the same reserve-then-complete ordering applies to the HLSL
// `register` spelling of an explicit binding. `autoBuf` is declared first
// but unannotated; CHECK asserts it is emitted as `register(u1)` because
// `u0` was already claimed by the annotated `pinnedBuf`.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> autoBuf;
RWStructuredBuffer<int> pinnedBuf : register(u0);

[numthreads(1, 1, 1)]
void main()
{
    autoBuf[0] = pinnedBuf[0];
}

// HLSL-DAG: autoBuf{{[A-Za-z0-9_]*}} : register(u1)
// HLSL-DAG: pinnedBuf{{[A-Za-z0-9_]*}} : register(u0)
