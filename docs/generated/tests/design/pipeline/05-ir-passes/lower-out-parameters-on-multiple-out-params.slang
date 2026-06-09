//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling
//META: doc_section_digest=bf314e51995b5a2b76e1104cf5f719db98f30e41b3a1ec2cddb5792a8ffcdf27
//META: purpose=Expansion: lower-out-parameters preserves the `out` keyword on multiple parameters (two `out` parameters in one helper) on HLSL — many-args axis to the existing single-out case.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Multi-out-parameter axis on lower-out-parameters. Two `out`
// parameters on the same helper survive on HLSL.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

void splitAndDouble(int n, out int evenPart, out int oddPart)
{
    evenPart = n & ~1;
    oddPart  = n & 1;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int e, o;
    splitAndDouble(a + int(tid.x), e, o);
    buf[0] = e * 10 + o;
}

// HLSL: out int{{.*}}, out int
