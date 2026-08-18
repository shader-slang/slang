//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#texture-and-image
//META: doc_section_digest=d256dde64890becaf8cd7de87d2e7c15cce6794507c618550a67824d086322d4
//META: purpose=`rwtex[i]` rvalue on an `RWTexture1D<T>` surfaces in `main` as a `call specialize(..., TextureShape1DType, ..., 1 : Int, ...)(%rwtex, %i)` — the doc's `imageSubscript` opcode lives inside the helper; the call form witnesses the 1D + RW combination.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `RWTexture1D<T>` element-read uses the `[]` operator; the helper
// is parameterized by 1D shape + isRW=1.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWTexture1D<float4> rwtex;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = rwtex[tid.x];
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShape1DType{{.*}})({{.*}}%rwtex{{.*}})
