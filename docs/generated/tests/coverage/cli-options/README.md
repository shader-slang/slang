---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T15:00:00+00:00
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: 83695ce7fd999921f0e228ef51b84e5f8201e91a6f81b5a9c90d9fa6a19da990
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/cli-options

White-box characterization tests targeting under-exercised command-line option
handling in `source/slang/slang-options.cpp` (~57% covered). These pin the
compiler's _current observed behaviour_ (not a spec). The manifest `source_doc`
is the options source file itself; the white-box target is also named in each
test's `//META: covers=` field. The `//META: doc_ref` points at the LLM-derived
design docs (`target-pipelines/{hlsl,spirv}.md`, `cross-cutting/{targets,diagnostics}.md`)
only to satisfy the shared `//META` shape — the authority for this tree is the
source, per `coverage/METHODOLOGY.md`.

## Intent

Drive slangc CLI flags from the hint area through `slangc` and pin the
_observable difference_ each flag makes in emitted output: matrix layout
(`-matrix-layout-row-major` / `-matrix-layout-column-major`),
`#line` emission (`-line-directive-mode none|glsl`), and the Vulkan flags
`-fvk-use-entrypoint-name`, `-fvk-invert-y`, `-fvk-bind-globals`. Each emission
test pairs the flag run against a no-flag (default) run with a distinct FileCheck
prefix so the option is the sole cause of the observed delta; the CHECK token was
copied verbatim from a real `slangc` run and re-confirmed by grepping the emitted
output. Each emission claim is fanned out across every shader text target that can
express it (`hlsl`, `glsl`, `spirv-asm`, `metal`, `wgsl`), one prefix per target,
because the same option reaches each back-end through a different emitter: the
`$Globals` slot chosen by `-fvk-bind-globals` surfaces as `layout(binding, set)`,
`register(bN, spaceN)`, `[[buffer(N)]]` and `@binding/@group`, and the matrix
layout surfaces as a pragma, a module-scope qualifier, a member decoration, or a
`_MatrixStorage_*` legalization struct. Targets on which an option produces no
observable delta at all are listed in `## Untested claims` rather than given a
vacuously-passing directive. Error-path option parsing (unknown option, bad `-O` value, bad `-line-directive-mode` value,
unknown `-profile`) uses `//DIAGNOSTIC_TEST` pinned to the exact `E####`; these
validate locally. Command-line diagnostics are attached to the synthetic command
line (source "line 0"), so the message-text matcher cannot anchor them
positionally — they are pinned by error code and the verbatim message is recorded
in each test's comment as the characterization record.

## Functional coverage

| Test                                                                                           | What it pins                                                                                                                                             | covers=                        |
| ---------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------ |
| [`matrix-layout-row-major-hlsl-pragma.slang`](matrix-layout-row-major-hlsl-pragma.slang)       | `-matrix-layout-row-major` makes HLSL emit `#pragma pack_matrix(row_major)`; default emits `column_major`.                                               | source/slang/slang-options.cpp |
| [`matrix-layout-spirv-decoration.slang`](matrix-layout-spirv-decoration.slang)                 | In SPIR-V the source layout emits the _inverted_ member decoration: `-matrix-layout-row-major` → `ColMajor`, `-matrix-layout-column-major` → `RowMajor`. | source/slang/slang-options.cpp |
| [`line-directive-mode-none-suppresses.slang`](line-directive-mode-none-suppresses.slang)       | `-line-directive-mode none` removes all `#line` directives from HLSL; default emits C-style `#line N "file"`.                                            | source/slang/slang-options.cpp |
| [`line-directive-mode-glsl-style.slang`](line-directive-mode-glsl-style.slang)                 | `-line-directive-mode glsl` emits GLSL numeric-file-id `#line N 0` directives.                                                                           | source/slang/slang-options.cpp |
| [`fvk-use-entrypoint-name-spirv.slang`](fvk-use-entrypoint-name-spirv.slang)                   | `-fvk-use-entrypoint-name` keeps the source entry name (`fragMain`) in `OpEntryPoint`; default renames it to `"main"`.                                   | source/slang/slang-options.cpp |
| [`fvk-invert-y-negates-position.slang`](fvk-invert-y-negates-position.slang)                   | `-fvk-invert-y` inserts an `OpFNegate` on the SV_Position.y output that is absent without the flag.                                                      | source/slang/slang-options.cpp |
| [`fvk-bind-globals-spirv-binding.slang`](fvk-bind-globals-spirv-binding.slang)                 | `-fvk-bind-globals 7 3` places `$Globals` at SPIR-V `Binding 7` / `DescriptorSet 3`; default is `0`/`0`.                                                 | source/slang/slang-options.cpp |
| [`unknown-option-diagnostic.slang`](unknown-option-diagnostic.slang)                           | An unrecognized command-line option is rejected with E00017.                                                                                             | source/slang/slang-options.cpp |
| [`invalid-optimization-level-diagnostic.slang`](invalid-optimization-level-diagnostic.slang)   | An out-of-range `-O9` is rejected with E00062 (unknown value for option).                                                                                | source/slang/slang-options.cpp |
| [`invalid-line-directive-mode-diagnostic.slang`](invalid-line-directive-mode-diagnostic.slang) | An unknown `-line-directive-mode` value is rejected with E00062.                                                                                         | source/slang/slang-options.cpp |
| [`unknown-profile-diagnostic.slang`](unknown-profile-diagnostic.slang)                         | An unknown `-profile` operand is rejected with E00014.                                                                                                   | source/slang/slang-options.cpp |

## Untested claims

Per-target rows recording the shader text targets an emission claim cannot be
fanned out to (`_claims.md` §2 "Meaningful back-ends"). Every other
target/claim pair carries a real `//TEST:SIMPLE` directive pinned to output
copied verbatim from `slangc`; nothing here was opted out to make a directive
green. The Claim cell repeats the test's `//META: purpose` verbatim.

| Claim                                                                                                                                | Reason                | Anchor                                                                              | Why untested                                                                                                                                                                                                                                                                                                                                      |
| ------------------------------------------------------------------------------------------------------------------------------------ | --------------------- | ----------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Verifies -fvk-invert-y inserts an OpFNegate on the SV_Position.y output that is absent without the flag.                             | unsupported-on-target | [#option-set-toggles](../../../design/target-pipelines/spirv.md#option-set-toggles) | Absent targets: hlsl , metal , wgsl . The Y-flip is a Vulkan-cross-API legalization applied only on Khronos targets; on these three the emitted text is byte-identical with and without the flag, so there is no delta a CHECK could pin. The Khronos pair (spirv-asm and glsl) both carry directives.                                            |
| Verifies -fvk-use-entrypoint-name keeps the source entry-point name in OpEntryPoint while the default renames it to "main".          | unsupported-on-target | [#option-set-toggles](../../../design/target-pipelines/spirv.md#option-set-toggles) | Absent targets: glsl , hlsl , metal , wgsl . The rename-to-`main` default lives in the SPIR-V OpEntryPoint naming path; GLSL must always name its entry `main` and the other three already emit the source name, so the flag makes no difference to any of the four (verified by a byte-level diff of flag vs no-flag output on the same source). |
| Verifies -line-directive-mode glsl emits GLSL-style numeric-file-id `#line N 0` directives rather than C-style `#line N "file"`.     | unsupported-on-target | [#option-set-toggles](../../../design/target-pipelines/hlsl.md#option-set-toggles)  | Absent target: spirv-asm . SPIR-V has no `#line` preprocessor construct at all — source locations ride on `OpLine` / `OpSource` and only under `-g` — so no `-line-directive-mode` value is observable there. glsl, metal and wgsl all carry directives.                                                                                          |
| Verifies -line-directive-mode none removes all `#line` directives from HLSL output while the default emits C-style `#line N "file"`. | unsupported-on-target | [#option-set-toggles](../../../design/target-pipelines/hlsl.md#option-set-toggles)  | Absent targets: wgsl , spirv-asm . WGSL emits no `#line` directives in its default mode, so `none` produces no observable delta and only a vacuously-true CHECK-NOT could be written; spirv-asm has no `#line` construct at all. glsl and metal carry default/none pairs.                                                                         |

## Unreachable gaps

| Option / area                                                              | Why not targeted                                                                                                                                                                                                                                                                                                                                                                               |
| -------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `-default-image-format-unknown` (OptionKind::DefaultImageFormatUnknown)    | Probed against both an unattributed `RWTexture2D<float4>` and a `[format("rgba8")]`-attributed one. Unattributed images already emit `OpTypeImage ... Unknown` by default, and the flag does not override an explicit `[format]` attribute — so no slangc input in scope produces an output that differs from the default. No observable delta to pin without a runner-gated path; not chased. |
| `-fvk-{b\|s\|t\|u}-shift <N> <space>` (OptionKind::VulkanBindShift)        | The two-trailing-operand parse consumed the input source-file path regardless of CLI ordering in this environment (compile produced empty output, exit 0), so a deterministic binding-delta CHECK could not be constructed. The sibling `-fvk-bind-globals` two-operand flag _was_ pinnable and is covered; the shift flag is left for a runner-validated follow-up.                           |
| `-capability <cap>` value parse                                            | `-capability bogus_cap` reports E00014 "unknown profile" — the capability operand flows through the same `Profile::lookUp` path as `-profile`, so its error is already characterized by `unknown-profile-diagnostic.slang`. A distinct _valid_-capability codegen effect is target/feature-specific (capability gating) and is better covered in the per-target design bundles.                |
| `-g<debug-level>` / `-g<debug-info-format>` (OptionKind::DebugInformation) | `-g2` does emit a distinguishable `NonSemantic.Shader.DebugInfo.100` import + `DebugSource` in SPIR-V vs `-g0`/default; however the same `-g2` output embeds the full shader source text and the absolute on-disk path in `OpString` operands, which is environment-dependent and noisy to pin deterministically. Left for a follow-up that can wildcard the volatile operands robustly.       |

## Doc gaps observed

| Anchor                                                                                                         | Kind                  | Gap                                                                                                                                                                                                                                                                                                                                                                                              | Suggested addition                                                                                                                                                                 |
| -------------------------------------------------------------------------------------------------------------- | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#option-set-toggles](../../../design/target-pipelines/spirv.md#option-set-toggles)                            | undocumented-behavior | The SPIR-V option-set-toggles section does not note that the source-level matrix layout selected on the CLI emits the _opposite_ SPIR-V member-decoration token (`-matrix-layout-row-major` → `ColMajor`, `-matrix-layout-column-major` → `RowMajor`) due to Slang's transposed matrix convention. A reader expecting `RowMajor` from `-matrix-layout-row-major` would be surprised.             | Add a one-line note to the matrix-layout toggle row: "the emitted SPIR-V decoration is the transpose of the source layout — row-major source emits `ColMajor`, and vice versa."    |
| [#error-codes-and-the-name-field](../../../design/cross-cutting/diagnostics.md#error-codes-and-the-name-field) | undocumented-behavior | Command-line option-parsing diagnostics (E00017, E00062, E00014) are attached to a synthetic "command line" source (line 0), not to any file in the translation unit. The diagnostics doc's rendering section does not mention this command-line virtual source, so a test author cannot predict that a position/caret-anchored annotation will not bind to it (only error-code matching works). | Add a short paragraph noting that CLI-parse diagnostics use a synthetic command-line `SourceView`; position-based matchers should pin them by error code, not by file line/column. |

## Sibling-bundle overlap

`-matrix-layout-*` emission shape is also exercisable from the target-pipeline
design bundles; this bundle pins it specifically as a _CLI-option-driven_ delta
(flag vs no-flag), which is the white-box target here (`slang-options.cpp`),
rather than as a target-codegen claim.
