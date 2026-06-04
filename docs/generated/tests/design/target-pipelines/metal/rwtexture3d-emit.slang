//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeimagesubscript
//META: doc_section_digest=dc38574e0fa7014bd162d7abd7e6c65c7f19c4c34a5ad5406321c1963ab37f95
//META: purpose=RWTexture3D<float4> emits as metal::texture3d<float, access::read_write> and legalizeImageSubscript rewrites subscript-store into .write() / subscript-load into .read().
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Metal arm of legalizeImageSubscript handles the 3D variant
// of RWTexture: a `tex[xyz] = v` subscript-store lowers to
// `tex.write(v, xyz)` and a subscript-load lowers to
// `tex.read(xyz)`. The IR-level texture type is
// texture3d<float, access::read_write>.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWTexture3D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[tid] = float4(1.0, 0.0, 0.0, 1.0);
    outBuf[tid.x] = tex[tid];
}

// CHECK: texture3d<float, access::read_write>
// CHECK: .write(
// CHECK: .read(
