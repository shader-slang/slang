//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#texture-and-image
//META: doc_section_digest=0b65d4084486d3b0e44a1d4ae82fad284cfe750fe170e9c06ae248dedd65891c
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
