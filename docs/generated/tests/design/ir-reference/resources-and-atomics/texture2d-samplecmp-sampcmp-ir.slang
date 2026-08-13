//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#sampling-and-combined-samplers
//META: doc_section_digest=a0ab646bb8b7c3f2d7e5acb9f418e3f9850e9bd5e2a629c46fdeeeb725e4652c
//META: purpose=`Texture2D::SampleCmp(sampCmp, uv, ref)` with a `SamplerComparisonState` surfaces in `main` as a `call specialize(%SampleCmp, ...)(%tex, %sampCmp, %uv, %ref)`, the comparison-sampler counterpart of the doc's `sample` opcode.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `SampleCmp` is the depth-comparison sampling form; it takes a
// `SamplerComparisonState` as the sampler operand and an extra
// reference scalar in the call. The library helper is parameterized
// by texture shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture2D<float> tex;
uniform SamplerComparisonState sampCmp;
uniform RWStructuredBuffer<float> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2 uv = float2(tid.xy);
    outbuf[0] = tex.SampleCmp(sampCmp, uv, 0.5);
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShape2DType{{.*}})({{.*}}%tex{{.*}}%sampCmp{{.*}})
