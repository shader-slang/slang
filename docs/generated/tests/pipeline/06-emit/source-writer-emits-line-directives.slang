//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#source-writer-abstraction
//META: doc_section_digest=153032ddf0c4d4832addf2be03aae534b965dbfa8ee7958964ccb5dd0fe90941
//META: purpose=SourceWriter::advanceToSourceLocation emits #line directives so downstream compilers can report errors at the user's source position. The directives appear in the HLSL, GLSL, Metal, CUDA, and C++ emit.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The SourceWriter (slang-emit-source-writer.h) advances to the
// user's source location and emits a `#line` directive so that
// downstream compilers (DXC, glslang, MSL compiler, NVCC, the host
// C++ compiler) report errors against the original Slang source.
// We compile the same shader to each text target that the doc lists
// as a SourceWriter consumer and FileCheck for a `#line` directive.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
    outBuf[tid.x + 1] = 2;
}

// HLSL: #line {{[0-9]+}}
// GLSL: #line {{[0-9]+}}
// METAL: #line {{[0-9]+}}
// CUDA: #line {{[0-9]+}}
// CPP: #line {{[0-9]+}}
