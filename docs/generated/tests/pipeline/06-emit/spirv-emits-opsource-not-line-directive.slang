//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#source-writer-abstraction
//META: doc_section_digest=153032ddf0c4d4832addf2be03aae534b965dbfa8ee7958964ccb5dd0fe90941
//META: purpose=Boundary/negative: SPIR-V emit does NOT produce a C `#line` directive — instead it emits `OpSource Slang 1` as the source-location anchor in the SPIR-V debug info section.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative-shape probe of the SourceWriter claim: the existing
// `source-writer-emits-line-directives.slang` pins `#line` on HLSL,
// GLSL, Metal, CUDA, and C++. SPIR-V is NOT in that list because the
// SPIR-V backend does not write a C-style `#line` directive — it
// uses the SPIR-V debug-info instruction `OpSource` instead. This
// test pins the negative shape (no `#line`) and the positive shape
// (`OpSource Slang`).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: OpSource Slang
// CHECK-NOT: #line
