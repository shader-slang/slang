//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=1784b8e9f30332a4995be817424580937cb7178f3cb7ce01ea908f8b8807e9fa
//META: purpose=Boundary: after const-folding, an `if (false)` arm is removed by DCE — the dead literal 9999 does not survive to the emit.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The SCCP / peephole / DCE pipeline collapses `if (false)` and
// removes the dead arm. The constant `9999` used only inside the
// dead arm therefore disappears from the emit text on every
// observable target.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int r;
    if (false) { r = 9999; } else { r = a; }
    buf[0] = r;
}

// HLSL: void main
// HLSL-NOT: 9999
// SPIRV: OpEntryPoint
// SPIRV-NOT: 9999
// GLSL: void main
// GLSL-NOT: 9999
