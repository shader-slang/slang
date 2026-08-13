//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#texture-and-image
//META: doc_section_digest=d256dde64890becaf8cd7de87d2e7c15cce6794507c618550a67824d086322d4
//META: purpose=`rwtex[xyz]` rvalue on an `RWTexture3D<T>` surfaces in `main` as a `call specialize(..., TextureShape3DType, ...)(%rwtex, %xyz)` — the 3D + RW witness of the doc's `imageSubscript` opcode in its `call`-form pre-inline shape.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 3D variant of the `[]`-on-RWTexture call shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWTexture3D<float4> rwtex;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = rwtex[tid];
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShape3DType{{.*}})({{.*}}%rwtex{{.*}})
