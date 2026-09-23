---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:12:24Z
target_doc: pipeline/04c-layout-ir.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 12
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 4
  escalated_to_finding: 0
---

# Gap-intake report for pipeline/04c-layout-ir.md

## Summary

Twelve gaps from two bundles. The eight reported by
`design/pipeline/04c-layout-ir` were all confirmed in source and fixed: three
diagnostic codes and message texts (`39012`, `39001`, `38010`) now sit next to
the `Diagnostics::` identifiers they belong to, three `missing-example` gaps got
minimal shaders lifted from the reporting tests, the option-gates table gained a
CLI-spelling column, and two `missing-surface` gaps about observability were
answered the way the source answers them — `createIRModuleForLayout` never calls
`dumpIR`, so neither the obfuscation strip nor a capability decoration can be
attributed to the layout module from compiler output. The four reported by
`coverage/type-layout` are deferred as one group: every one of them asks for
type-layout *rule* material owned by `source/slang/slang-type-layout.cpp`, which
is not in this page's `watched_paths` and has no owning page in
`manifest.yaml`. Nothing was escalated; no gap turned out to be a compiler
defect. The page is now 32,330 bytes against a 32,768-byte cap, which is the
practical reason the fixes are terse.

Two fixes say more than the gap asked. The bindless stage-7 row records that the
`39012` warning fires only when `-bindless-space-index` was given explicitly
(`hasOption`), which the gap did not mention, and the ray-tracing note
enumerates the CUDA family's partial support (`getCallablePayloadParameterRules`
alone returns `nullptr`) rather than only the all-or-nothing families the gap
named.

## Actions

| Gap ID       | Action   | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                       | Fix summary                                                                                                                     |
| ------------ | -------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| b1d70bb86164 | fixed    | Dump spelling confirmed as `requireCapabilityAtom` in the op-info table generated from `source/slang/slang-ir-insts.lua:1909-1913` (`build/source/slang/fiddle/slang-ir-insts-info.cpp.fiddle`), printed by `dumpIRDecorations` as `[<mnemonic>(...)]` (`source/slang/slang-ir.cpp:7905-7913`). The "no observation point" half is confirmed in watched `source/slang/slang-lower-to-ir.cpp`: the only `dumpIR` calls in the file are at 15541 and 15797; `createIRModuleForLayout` (16381) has none. | named `IRRequireCapabilityAtomDecoration` / `[requireCapabilityAtom(...)]` in the Caveats bullet and stated there is no layout-module observation point |
| a82648ac3cd2 | fixed    | Null-returning families read directly: `source/slang/slang-type-layout.cpp:2280-2291` (CPU), `2402-2413` (LLVM), `2835-2848` (Metal), `2553-2556` (CUDA callable only); non-null at `2130-2143` (GLSL), `2222-2234` (HLSL), `2927-2939` (WGSL). Target-to-family mapping at `2955-3010` puts `-target cpp` on the CPU family and `-target metal` on the Metal family. Shader shape matches the bundle test `no-layout-ir-module-when-binding-fails.slang`.                                          | added a four-line `miss` entry point and the per-family `nullptr` list under Diagnosed failures during binding                       |
| 9667530c34b5 | fixed    | Warning `38010` = `unhandled-mod-on-entry-point-parameter` at `source/slang/slang-diagnostics.lua:4252-4257`, emitted at `source/slang/slang-check-shader.cpp:2341` behind `isVkBindingCompatibleEntryPointParameterType` (`:920`). Honored/ignored outcomes are the verified CHECK lines of the bundle tests `vk-binding-entry-point-param-honored-on-khronos-and-wgpu.slang` (`Binding 3` / `DescriptorSet 1`) and `vk-binding-entry-point-param-ignored-on-other-targets.slang` (`register(t0)`). | added a two-line annotated entry point with its SPIR-V and HLSL outcomes, and named warning `38010` in the ignorable-annotation paragraph |
| 48c86999fd4a | deferred | The AoS-to-SoA reshape and `maybeAdjustLayoutForArrayElementType` live in `source/slang/slang-type-layout.cpp`, which is not in this page's `watched_paths` and is not owned by any key in `docs/generated/design/_meta/manifest.yaml`. Deferred with the three sibling `coverage/type-layout` gaps: they want one new "layout rules" section (or peer page) rather than four scattered notes, and the page has under 500 bytes of headroom under its 32,768-byte cap. Needs a manifest decision first. | —                                                                                                                                   |
| 37877001e22b | deferred | The `ConstantBuffer<T, L>` surface itself *is* confirmable in a watched path (`source/slang/hlsl.meta.slang:74`, `115-121`, and `IBufferDataLayout` implementors at `30-70`), but the gap asks for it "in the layout-rule table" together with the offsets each argument selects, and neither the table nor its backing source (`slang-type-layout.cpp`) is available to this page. Deferred with the sibling `coverage/type-layout` gaps.                                                          | —                                                                                                                                   |
| ca41e2ef5b56 | deferred | The container-kind-to-rule-set mapping and the `-fvk-use-dx-layout` / `-force-glsl-scalar-layout` swaps are decided in `source/slang/slang-type-layout.cpp` (target family dispatch at `2955-3010`, per-family `getConstantBufferRules` / `getStructuredBufferRules`), outside this page's `watched_paths`. This is the largest of the four and the one the other three hang off. Deferred pending a `watched_paths` expansion or a new page.                                                        | —                                                                                                                                   |
| ddba6f169a35 | deferred | The `Optional<T>` niche-vs-tuple split and the `(T, bool)` vs `(bool, T)` note are in the `OptionalType` arm of `_createTypeLayout` in `source/slang/slang-type-layout.cpp`, outside this page's `watched_paths`. Deferred with the sibling `coverage/type-layout` gaps.                                                                                                                                                                                            | —                                                                                                                                   |
| d5588b5924ad | fixed    | CLI spellings confirmed in the option table at `source/slang/slang-options.cpp:839-842` (`-obfuscate`) and `:922-925` (`-bindless-space-index <index>`); the watched `source/slang/slang-compiler-options.h` carries only the accessors (`:361`), so the spellings genuinely are not reachable from the header.                                                                                                                                                     | added a CLI-spelling column to the option-gates table, cited `slang-options.cpp`, and added it to Manifest coverage                  |
| 88573dbd3edf | fixed    | Confirmed in watched `source/slang/slang-lower-to-ir.cpp`: `createIRModuleForLayout` (16381-16548) contains no `dumpIR` call, and the executable module's strip block sets `stripOptions.shouldStripNameHints = linkage->m_optionSet.shouldObfuscateCode()` at `:15748` — the same gate — so `-obfuscate` strips both modules together. Took the gap's second option ("say explicitly that the difference is not observable").                                       | added a paragraph stating the block has no observation point outside the compiler                                                   |
| 680a127c66e6 | fixed    | Warning `39012` = `requested-bindless-space-index-unavailable` at `source/slang/slang-diagnostics.lua:4696-4701`; the scan-and-place code is watched `source/slang/slang-parameter-binding.cpp:4818-4834`, which also shows the warning is suppressed unless `hasOption(CompilerOptionName::BindlessSpaceIndex)` and that `bindlessSpaceIndex` is set to the space actually chosen. Matches the bundle test `bindless-space-index-unavailable-warning.slang`.        | put the `39012` code and message text plus the explicit-option condition into the stage-7 row                                       |
| 32e51a777983 | fixed    | Warning `39001` = `parameter-bindings-overlap`, "explicit binding overlap", at `source/slang/slang-diagnostics.lua:4653-4659`, emitted from watched `source/slang/slang-parameter-binding.cpp:986` off the `usedResourceRanges[...].Add` result recorded at `:952` — immediately after the `InputAttachmentIndex` guard at `:947`. Example shape and diagnostic text are the CHECK lines of `input-attachment-index-overlap-still-diagnosed.slang`.                 | named warning `39001` in the used-range sentence and added the two-parameter shared-index example                                    |
| cd1b5387a8ae | fixed    | Confirmed in watched `source/slang/slang-lower-to-ir.cpp:15793-15802`: `generateIRForTranslationUnit` emits the `### LOWER-TO-IR:` label (`dumpIR(..., "LOWER-TO-IR", ...)`), and no later dump site exists on the failed-binding path since `getOrCreateLayout` returns before construction. The absent tokens are the verified CHECK-NOTs of `no-layout-ir-module-when-binding-fails.slang`.                                                                       | added the `-dump-ir` observation to the "Nothing is built when binding failed" bullet                                               |
