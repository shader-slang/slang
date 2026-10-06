//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=1784b8e9f30332a4995be817424580937cb7178f3cb7ce01ea908f8b8807e9fa
//META: purpose=Boundary: the init-local-var pass tolerates an uninitialized struct local — the function compiles successfully and the live field is read back correctly on HLSL.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Edge for `slang-ir-init-local-var.cpp`: a struct local that is
// never assigned before partial write to its `.x` field. The pass
// inserts default initialization so the read of `.x` succeeds in
// the emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

struct Box { int x; };

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Box b;
    b.x = a;
    buf[0] = b.x;
}

// HLSL: void main
