//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=1784b8e9f30332a4995be817424580937cb7178f3cb7ce01ea908f8b8807e9fa
//META: purpose=The init-local-var pass inserts default initialization for locals so the SPIR-V emit does not need explicit zero-init for the typical local in a Slang shader.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Init local var" row of the cleanup category
// (`slang-ir-init-local-var.cpp`) inserts default initialization
// for locals. After the pass runs, a freshly-defaulted struct
// local can be read without triggering the use-uninitialized check.
// (We observe the absence of an error.)

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

struct Pair { int a; int b; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Pair p = {};
    buf[0] = p.a + p.b;
}

// HLSL: void main
