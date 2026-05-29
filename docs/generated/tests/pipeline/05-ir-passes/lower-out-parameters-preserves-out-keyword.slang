//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
//META: purpose=The lower-out-parameters pass routes an `out` parameter through a pointer-like address on every text-emit target.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Lower out parameters" row of the type-and-value legalization
// table (`slang-ir-lower-out-parameters.cpp`) implements ABI
// translation of `out` parameters. The HLSL emit retains the `out`
// keyword on the parameter; the GLSL emit also preserves `out`; the
// CUDA / C++ emit uses a pointer / reference for the same parameter.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

void produce(out int o)
{
    o = a + 1;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int r;
    produce(r);
    buf[0] = r;
}

// HLSL: out int
// GLSL: out int
