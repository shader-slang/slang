//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#texture-and-image
//META: doc_section_digest=9181b1e074afdf780bb1918cb592389fdbe5dee019af40f1c48f4b6cb9064e6b
//META: purpose=`Texture1D::Load(int2(off, lod))` surfaces in `main` as a `call specialize(%Load, ..., TextureShape1DType, ...)(%tex, %coord)`. The doc's `imageLoad` opcode itself lives inside the helper body; only the call form is observable from `main`.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 1D variant of the `Load` `call specialize(...)` shape; the
// underlying `imageLoad` opcode is inlined later in the pipeline
// (see existing bundle's "Doc gaps observed").

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

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
