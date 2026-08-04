//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#option-set-toggles
//META: doc_section_digest=730d07f4cad01d2edbbbf03523fae836c3254cc356ffd1ac578d0a21bad19381
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

//TEST:SIMPLE(filecheck=KEEP):-target spirv-asm -fvk-use-entrypoint-name -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=RENAME):-target spirv-asm -entry fragMain -stage fragment

[shader("fragment")]
float4 fragMain() : SV_Target
{
    return float4(0, 0, 0, 1);
}

//KEEP: OpEntryPoint Fragment %fragMain "fragMain"
//RENAME: OpEntryPoint Fragment %fragMain "main"
