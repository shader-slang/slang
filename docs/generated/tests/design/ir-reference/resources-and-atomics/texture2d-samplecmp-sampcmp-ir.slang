//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#sampling-and-combined-samplers
//META: doc_section_digest=bbf71e9879e5ca6464e703729e122f9e3af7f79c62a7f182ca2c1576a2edbfe2
//META: purpose=`Texture2D::SampleCmp(sampCmp, uv, ref)` with a `SamplerComparisonState` surfaces in `main` as a `call specialize(%SampleCmp, ...)(%tex, %sampCmp, %uv, %ref)`, the comparison-sampler counterpart of the doc's `sample` opcode.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `SampleCmp` is the depth-comparison sampling form; it takes a
// `SamplerComparisonState` as the sampler operand and an extra
// reference scalar in the call. The library helper is parameterized
// by texture shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform Texture2D<float> tex;
uniform SamplerComparisonState sampCmp;
uniform RWStructuredBuffer<float> outbuf;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2 uv = float2(tid.xy);
    outbuf[0] = tex.SampleCmp(sampCmp, uv, 0.5);
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShape2DType{{.*}})({{.*}}%tex{{.*}}%sampCmp{{.*}})
