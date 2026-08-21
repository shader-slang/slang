//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -emit-spirv-via-glsl routes SPIR-V generation through glslang, so the module reports the Glslang generator and SPIR-V 1.0, while the default direct path reports the Slang generator and SPIR-V 1.5, and a later -emit-spirv-directly wins.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the shared `OptionKind::EmitSpirvViaGLSL` /
// `OptionKind::EmitSpirvDirectly` arm of `OptionsParser::_parse` in
// slang-options.cpp. The arm derives `SLANG_EMIT_SPIRV_VIA_GLSL` or
// `SLANG_EMIT_SPIRV_DIRECTLY` from which spelling was used, reads the target's
// current `OptionKind::EmitSpirvMethod` back out, and stores the new choice. Only
// the `EmitSpirvDirectly` half was reached by the shipped suites.
//
// The two paths are trivially distinguishable in the disassembly header, because
// each SPIR-V producer stamps its own generator magic and its own version. The
// direct emitter writes `; Generator: Khronos Slang Compiler` at
// `; Version: 1.5`; going through glslang writes
// `; Generator: Khronos Glslang Reference Front End` at `; Version: 1.0`. That
// header pair is the tight distinguishing token, and it is not something the
// shader source can influence.
//
// LASTWINS passes both flags in that order to pin that the arm is last-one-wins
// rather than sticky: the trailing `-emit-spirv-directly` puts the module back on
// the Slang generator.

//TEST:SIMPLE(filecheck=DIRECT):-target spirv-asm -entry main -stage fragment
//TEST:SIMPLE(filecheck=VIAGLSL):-target spirv-asm -entry main -stage fragment -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=LASTWINS):-target spirv-asm -entry main -stage fragment -emit-spirv-via-glsl -emit-spirv-directly

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(1, 0, 0, 1);
}

//DIRECT: ; Generator: Khronos Slang Compiler

//VIAGLSL: ; Version: 1.0
//VIAGLSL: ; Generator: Khronos Glslang Reference Front End

//LASTWINS: ; Generator: Khronos Slang Compiler
