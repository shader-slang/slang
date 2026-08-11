//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeimagesubscript
//META: doc_section_digest=1947215d93f0e5f1e4204f6fdb27c6177329ff0d8cfba1886b1c651160781409
//META: purpose=`RWTexture2D<T>` emits as `texture2d<T, access::read_write>` and `legalizeImageSubscript` rewrites subscript-store into `.write(...)` and subscript-load into `.read(...)`.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies why Metal is in the image-subscript arm at all: MSL has no
// generic subscript operator on a texture, only member `read` / `write`
// calls. The kernel both stores through and loads from the same texture,
// so a single test observes both rewrites; CHECK asserts the read_write
// access qualifier on the texture type plus a `.write(` and a `.read(`
// call in the body.

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
