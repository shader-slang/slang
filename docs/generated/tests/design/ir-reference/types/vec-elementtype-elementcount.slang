//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=A vector value is typed Vec(elementType, elementCount) in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "`Vec` and `Mat`" note says
// "`Vec(elementType, elementCount)` is the IR encoding of HLSL
// `vector<T,N>`." A `uint3` parameter and a `float4` local both
// show up with their explicit `Vec(...)` operand pair in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<float> buf;
uniform float4 v;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = v.x;
}

// IR-LABEL: func %main
// IR: param %tid {{.*}} Vec(UInt, 3 : Int)
// IR: let {{.*}}%v{{.*}}: Vec(Float, 4 : Int){{.*}}= global_param
