//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=bf314e51995b5a2b76e1104cf5f719db98f30e41b3a1ec2cddb5792a8ffcdf27
//META: purpose=Boundary: a completely dead two-function call chain (`deadOuter` calls `deadInner`, neither reachable) is fully removed by DCE — neither name appears in emit.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two top-level helpers form a dead call chain: `deadOuter` calls
// `deadInner`, but nothing calls `deadOuter`. The cleanup category's
// `eliminateDeadCode` strips both, not just the leaf.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int deadInner(int v) { return v * 7; }
int deadOuter(int v) { return deadInner(v) + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// HLSL: void main
// HLSL-NOT: deadInner
// HLSL-NOT: deadOuter
// SPIRV: OpEntryPoint
// SPIRV-NOT: deadInner
// SPIRV-NOT: deadOuter
