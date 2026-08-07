//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#texture-and-image
//META: doc_section_digest=d256dde64890becaf8cd7de87d2e7c15cce6794507c618550a67824d086322d4
//META: purpose=`Texture2DMS<T>::Load(coord, sampleIndex)` surfaces in `main` as a `call specialize(%Load, ..., TextureShape2DType, ...)(%tex, %coord, %sampleIndex)`, where the `sampleIndex` is passed as a trailing operand to the helper. The MS-ness is carried in the `global_param` texture type, but the `Load`-helper specialization picks the 2D shape token here.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Texture2DMS::Load` has an explicit `sampleIndex` argument; the
// doc's `imageLoad` row mentions an "auxCoord1" operand that
// corresponds to the sample index.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture2DMS<float4> tex;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int2 coord = int2(tid.xy);
    outbuf[0] = tex.Load(coord, 0);
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShape2DType{{.*}})({{.*}}%tex{{.*}})
