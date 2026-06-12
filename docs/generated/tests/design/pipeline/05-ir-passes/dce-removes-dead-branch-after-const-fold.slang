//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=bf314e51995b5a2b76e1104cf5f719db98f30e41b3a1ec2cddb5792a8ffcdf27
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
