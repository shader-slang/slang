//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-02T10:45:10+00:00
//META: source_commit=6815ed82d5e8c95b0e1258ca2eba0ca9c7825d86
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
//META: purpose=The SourceWriter emits #line directives that reference the HLSL meta-module when emitted code originates from there.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a built-in whose body lives in `hlsl.meta.slang` causes the
// `SourceWriter` to emit `#line` directives that point at the meta-module
// source. A double-precision `fma` keeps its meta-module body in the emitted
// HLSL. Combined-sampler `SampleLevel`, which this test used to use, now
// lowers straight to the texture/sampler pair and carries no meta-module
// `#line`, and no built-in tried since still emits one for `core.meta.slang`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<double> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    double d = double(tid.x);
    outBuf[tid.x] = fma(d, d, d);
}

// CHECK: #line {{[0-9]+}} "{{.*}}hlsl.meta.slang"
