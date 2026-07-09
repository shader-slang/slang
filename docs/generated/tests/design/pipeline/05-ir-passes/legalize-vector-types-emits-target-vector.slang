//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=87cc303db4e3bb3825cd2dfa57f040c4ffad9180d465ffc6172dea9a41fbd518
//META: purpose=The legalize-vector-types pass routes Slang `vector<T, N>` into target-specific vector spellings (`int3` on HLSL, `ivec3` on GLSL).
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Legalize vector types" row of the type-and-value legalization
// table (`slang-ir-legalize-vector-types.cpp`) renders Slang
// vectors as the target's native vector type. `vector<int, 3>` is
// `int3` on HLSL and `ivec3` on GLSL.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform RWStructuredBuffer<vector<int, 3> > buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    vector<int, 3> v = vector<int, 3>(int(tid.x), int(tid.y), int(tid.z));
    buf[0] = v;
}

// HLSL: int3
// GLSL: ivec3
