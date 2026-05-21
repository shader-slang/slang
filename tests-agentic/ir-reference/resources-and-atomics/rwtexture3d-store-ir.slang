//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#texture-and-image
//META: doc_section_digest=0b65d4084486d3b0e44a1d4ae82fad284cfe750fe170e9c06ae248dedd65891c
//META: purpose=`rwtex[xyz] = val` assignment on an `RWTexture3D<T>` surfaces in `main` as a `call specialize(..., TextureShape3DType, ...)(%rwtex, %xyz, %val)` — the 3D witness of the doc's `imageStore` opcode in its pre-inline `call` shape.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 3D variant of the `[]=`-on-RWTexture call shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWTexture3D<float4> rwtex;
uniform RWStructuredBuffer<int> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    rwtex[tid] = float4(1.0, 2.0, 3.0, 4.0);
    outbuf[0] = 1;
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShape3DType{{.*}})({{.*}}%rwtex{{.*}})
