//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#texture-and-image
//META: doc_section_digest=0b65d4084486d3b0e44a1d4ae82fad284cfe750fe170e9c06ae248dedd65891c
//META: purpose=`Texture1D::Load(int2(off, lod))` surfaces in `main` as a `call specialize(%Load, ..., TextureShape1DType, ...)(%tex, %coord)`. The doc's `imageLoad` opcode itself lives inside the helper body; only the call form is observable from `main`.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 1D variant of the `Load` `call specialize(...)` shape; the
// underlying `imageLoad` opcode is inlined later in the pipeline
// (see existing bundle's "Doc gaps observed").

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture1D<float4> tex;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int2 coord = int2((int)tid.x, 0);
    outbuf[0] = tex.Load(coord);
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShape1DType{{.*}})({{.*}}%tex{{.*}})
