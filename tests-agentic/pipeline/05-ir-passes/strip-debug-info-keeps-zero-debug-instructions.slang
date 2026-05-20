//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=bf314e51995b5a2b76e1104cf5f719db98f30e41b3a1ec2cddb5792a8ffcdf27
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
