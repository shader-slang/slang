//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#source-location-serialization
//META: doc_section_digest=0b9df2a75e79b7e8b6026192b354673f12b076885b33df7d9612c34bd2b3f441
//META: purpose=Each source location keeps its originating file path and line number all the way to emit, appearing as a #line directive naming this file on every C-family text target and as an OpSource marker on SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the per-file record a source location is resolved
// against - its path and its line-start table - is still intact at the
// end of the pipeline, and is target-dependent in shape, so every
// text-emit backend is exercised. Each directive compiles the same
// compute kernel and its prefix asserts a `#line <n> "<this file>"`
// directive: matching both a numeric line and the originating filename
// is what shows the location was resolved back to a file rather than
// reduced to an opaque integer. SPIR-V has no #line, so the SPIRV
// prefix asserts the OpSource marker the module header carries instead.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 42;
}

// HLSL: #line {{[0-9]+}} "{{.*source-loc-line-directives-multi-target\.slang}}"
// GLSL: #line {{[0-9]+}} "{{.*source-loc-line-directives-multi-target\.slang}}"
// METAL: #line {{[0-9]+}} "{{.*source-loc-line-directives-multi-target\.slang}}"
// CUDA: #line {{[0-9]+}} "{{.*source-loc-line-directives-multi-target\.slang}}"
// CPP: #line {{[0-9]+}} "{{.*source-loc-line-directives-multi-target\.slang}}"
// WGSL: #line {{[0-9]+}} "{{.*source-loc-line-directives-multi-target\.slang}}"
// SPIRV: OpSource
