//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=The SourceWriter emits #line directives that reference both the user .slang file and the core/HLSL meta-modules when emitted code originates from there.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a built-in that draws from `hlsl.meta.slang` /
// `core.meta.slang` causes the `SourceWriter` to emit `#line`
// directives that point at the meta-module sources.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Sampler2D combined;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = combined.SampleLevel(float2(0.5, 0.5), 0);
}

// CHECK-DAG: #line {{[0-9]+}} "{{.*}}core.meta.slang"
// CHECK-DAG: #line {{[0-9]+}} "{{.*}}hlsl.meta.slang"
