//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeimagesubscript
//META: doc_section_digest=25850c290ac50a3e82687e98c46c2de0d091ae0c4c7fe779c2a1f56a5456831e
//META: purpose=RWTexture2D<T> emits as texture2d<T, access::read_write> and legalizeImageSubscript rewrites subscript-store into .write() and subscript-load into .read().
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `legalizeImageSubscript` runs in the Metal/GLSL/SPIR-V arm.
// On Metal, RWTexture2D<float4> emits as
// `texture2d<float, access::read_write>` and a subscript-store
// `tex[xy] = v` becomes `tex.write(v, xy)`; subscript-load
// becomes `tex.read(xy)`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWTexture2D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[tid.xy] = float4(1.0, 0.0, 0.0, 1.0);
    outBuf[tid.x] = tex[tid.xy];
}

// CHECK: texture2d<float, access::read_write>
// CHECK: .write(
// CHECK: .read(
