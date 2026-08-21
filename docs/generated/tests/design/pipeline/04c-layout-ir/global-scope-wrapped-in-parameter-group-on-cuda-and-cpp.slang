//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=cb9b7a613d39760a93548dfbce14e96da09e14aa0f49313bd097e522bedd9206
//META: purpose=On CUDA and C++ the global scope is itself a parameter group, so the module-level layout is a parameterGroupTypeLayout even without a user-written cbuffer.
//META: intent=boundary
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the wrapped/unwrapped global-scope branch is target-driven, not
// only syntax-driven. The same source with no `cbuffer` produces a plain
// `structTypeLayout` on SPIR-V but a `parameterGroupTypeLayout` on CUDA and
// C++, where global uniforms are gathered into a generated parameter block.
// The SPV prefix pins the unwrapped shape, CUDA/CPP the wrapped one.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// SPV-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// SPV: {{^}}varLayout(
// SPV-NEXT: structTypeLayout(

// CUDA-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CUDA: {{^}}varLayout(
// CUDA-NEXT: parameterGroupTypeLayout(

// CPP-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CPP: {{^}}varLayout(
// CPP-NEXT: parameterGroupTypeLayout(
