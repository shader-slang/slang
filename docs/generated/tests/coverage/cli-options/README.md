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
copied verbatim from a real `slangc` run at `source_commit` and re-confirmed by
grepping the emitted output (FileCheck is absent locally, so the SIMPLE
directives report `ignored` on verify — CI validates them). Error-path option
parsing (unknown option, bad `-O` value, bad `-line-directive-mode` value,
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
