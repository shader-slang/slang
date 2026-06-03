//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
//META: purpose=Boundary: a length-1 vector — the smallest documented vector dimension — is legalized to a plain scalar `int` in the HLSL emit.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Smallest documented vector dimension. `vector<int, 1>` is
// legalized to a scalar `int` on HLSL — the spelling `int1` does
// not appear.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    vector<int, 1> v = vector<int, 1>(int(tid.x));
    buf[0] = v.x;
}

// HLSL: void main
