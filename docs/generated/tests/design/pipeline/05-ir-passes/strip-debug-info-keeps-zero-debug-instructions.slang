//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=1784b8e9f30332a4995be817424580937cb7178f3cb7ce01ea908f8b8807e9fa
//META: purpose=The strip-debug-info pass removes debug-only instructions when debug info is not requested, so the SPIR-V emit only carries minimal OpSource debug info.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Strip debug info" row of the SSA / cleanup category
// (`slang-ir-strip-debug-info.cpp`) removes debug instructions when
// debug info is not requested. Without `-g`, the SPIR-V emit lacks
// `OpLine` / `OpDebugInfo*` instructions even though `OpSource`
// remains as a minimal preamble entry.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// SPIRV: OpEntryPoint
// SPIRV-NOT: OpLine
