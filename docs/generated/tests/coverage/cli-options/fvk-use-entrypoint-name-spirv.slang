//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#option-set-toggles
//META: doc_section_digest=51a3cfba2e9b446866969702a281584b197e5b4c38e04278312f866f3a25794f
//META: purpose=Verifies -fvk-use-entrypoint-name keeps the source entry-point name in OpEntryPoint while the default renames it to "main".
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of OptionKind::VulkanUseEntryPointName (-fvk-use-entrypoint-name).
// By default the SPIR-V emitter rewrites the entry-point's exported name to
// "main"; the flag preserves the source name. With a source entry named
// `fragMain`, the KEEP directive pins OpEntryPoint exporting `"fragMain"`, and
// the RENAME directive (no flag) pins the default `"main"` export — both on the
// same %fragMain function id, isolating the option's effect.

// Emission fan-out: the rename-to-"main" default and the flag that suppresses
// it live in the SPIR-V emitter's OpEntryPoint naming path only. On this same
// source, `-fvk-use-entrypoint-name` produces byte-identical output with and
// without the flag for glsl, hlsl, metal and wgsl (GLSL must always name its
// entry `main` regardless; the other three already keep `fragMain`), so there
// is no delta a CHECK could pin. Those four targets are recorded in the bundle
// README's Untested-claims table with reason `unsupported-on-target` rather
// than given a vacuously-passing directive.

//TEST:SIMPLE(filecheck=KEEP):-target spirv-asm -fvk-use-entrypoint-name -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=RENAME):-target spirv-asm -entry fragMain -stage fragment

[shader("fragment")]
float4 fragMain() : SV_Target
{
    return float4(0, 0, 0, 1);
}

//KEEP: OpEntryPoint Fragment %fragMain "fragMain"
//RENAME: OpEntryPoint Fragment %fragMain "main"
