---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-12T06:38:51Z
target_doc: cross-cutting/targets.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 67149d1e03ebf1d4645ddd224ff4647a8ea5db53
gap_count: 12
actions:
  fixed: 9
  rejected_bogus: 0
  rejected_out_of_scope: 3
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for cross-cutting/targets.md

## Summary

This is a re-run of the intake pass, and one verdict changed:
`ba43401fe07f` moves from `deferred` to `fixed`. The previous cycle
deferred it on the grounds that no host-native `slangc` existed and
that the call site lay outside `watched_paths`; both grounds are gone
— `build-arm64/Debug/bin/slangc` is an arm64 build of the current
`HEAD`, and `source/slang/slang-options.cpp`,
`source/slang/slang-compiler.h` and
`source/slang/slang-profile-defs.h` are now watched. Running the
compiler settled the gap and also corrected the document's account of
the carve-out: the cross-family case is not suppressed by the folding
function's early return but by a family gate ahead of it, because a
cross-family profile *does* supply a SPIR-V version atom. The
resulting breakdown is nine fixed, three rejected as out-of-scope,
none deferred, and nothing escalated as a compiler defect. The other
eleven verdicts are carried forward unchanged, and their Evidence
verbatim except for three now-false clauses: `d25f710d2ffa` and
`70c758b923f2` each said their authoritative table was outside
`watched_paths`, which the expansion has made untrue, and
`a36b7c8757cc` gains a compiler re-confirmation of the `; Version:`
lines it rests on. No section other than
`### Profiles versus explicit -capability` was touched, so only that
section's digest moves. The previous pass's two operator items are
both resolved: the size cap is now 40960 against a 28270-byte
document, and the three unwatched files it named are in
`watched_paths`.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 4ca278c8d71b | fixed | `source/slang/hlsl.meta.slang:14130-14132` declares `[require(abort)] void abort<each T>(NativeString format, expand each T args);`; `source/slang/slang-capabilities.capdef:2476` is `alias abort = GL_EXT_shader_abort;` | named the `abort` builtin and its call form in the `[Compound]` paragraph |
| c15c7c4c2e96 | fixed | `source/slang/hlsl.meta.slang:22376, 22404, 22630, 22655` (sphere) and `22432, 22479, 22526, 22552, 22578, 22604` (LSS), each preceded by `[require(glsl_hlsl_spirv, rayquery_*_nv)]` and `[__requiresNVAPI]` | listed the ten gated `RayQuery` methods and the NVAPI requirement, in the same `[Compound]` edit as 4ca278c8d71b |
| c0d54084a05b | fixed | `source/slang/slang-compiler.h:239-244` (`maybeDiagnose` drops `Capability`-category diagnostics under `IgnoreCapabilities`) and `:255-264` (`maybeDiagnoseWarningOrError` picks the error form under `RestrictiveCapabilityCheck`); the two printed forms are the verified `CHECK-ERR` / `CHECK-WARN` lines of `docs/generated/tests/design/cross-cutting/targets/restrictive-capability-check-rejects-missing-atom.slang` | added a paragraph under `### Runtime representation` naming both flags and the diagnostic each selects |
| a2ab094d2c29 | fixed | `source/slang/slang-capabilities.capdef:683` declares `def SPV_KHR_cooperative_matrix : _spirv_1_6 + SPV_EXT_physical_storage_buffer + SPV_KHR_vulkan_memory_model;`; the emitted rendering is the verified `SPIRV-DAG` lines of `docs/generated/tests/design/cross-cutting/targets/capability-extension-inherits-version-floor.slang` | added the `-target spirv-asm -capability SPV_KHR_cooperative_matrix` worked example to the version-floor paragraph |
| a9fa50665328 | rejected-out-of-scope | The document's `## What is not in this document` bullet assigns "how a particular construct is spelled in the emitted language" to `docs/generated/design/target-pipelines/`, adding "even when the reason is a capability". A per-family statement of where a version word appears in emitted SPIR-V / GLSL / HLSL / MSL is that form. | — |
| c402197b09a5 | fixed | `source/slang/core.meta.slang:1076-1087` is a `__target_switch` with a `case hlsl:` arm and a `default:` arm; fall-through and the no-arm outcome are the verified `CHECK` lines of `docs/generated/tests/design/cross-cutting/targets/target-switch-missing-arm-falls-to-default.slang` and `target-switch-missing-arm-rejected.slang` (the entry point, not the switch, is rejected) | added a minimal `__target_switch` example and the no-matching-arm rule under **Specialization passes** |
| ba43401fe07f | fixed | Settled by running the compiler; verdict changed from `deferred` (the previous cycle's "no host `slangc`" blocker is gone, and `source/slang/slang-options.cpp` is now watched). Diagnosed triple: `build-arm64/Debug/bin/slangc -O0 -target spirv -profile spirv_1_3 -capability SPV_KHR_cooperative_matrix -entry main -stage compute t.slang -o t.spv` exits 1 printing `error[E00046]: a requested '-capability' requires a higher target version than the explicitly requested profile 'spirv_1_3'; specify a higher '-profile' or remove the conflicting '-capability'` — the message of `source/slang/slang-diagnostics.lua:342-346`, raised at `source/slang/slang-options.cpp:4530-4537`, with the `_spirv_1_6` floor at `source/slang/slang-capabilities.capdef:683`. The same line at `-profile spirv_1_6`, and with no `-profile`, exits 0 and writes a module whose header version word is 1.6. Counter-example: swapping in `-profile glsl_450` or `-profile sm_6_0` also exits 0 and emits 1.6, while each of those profiles alone emits 1.3 — so the suppression is *not* the function's `selectedVersion == CapabilityAtom::Invalid` early return (`source/slang/slang-capability.cpp:217-220`) but the call site's family gate (`source/slang/slang-options.cpp:4486-4508`), whose comment at `:4480-4483` names `-profile glsl_450` plus `-capability spirv_1_4` as the false positive it exists to avoid. | added the diagnosed `-profile spirv_1_3` + `SPV_KHR_cooperative_matrix` command line with its verbatim `E00046` text, plus the cross-family counter-example, attributing its silence to the call site's family gate rather than to the folding function's early return |
| 34b5fafdf292 | fixed | `source/slang/slang-options.cpp:3306-3312` reports `Diagnostics::UnknownProfile` for a `-capability` atom that `findCapabilityName` rejects; `:2671-2677` is the `Profile::lookUp` path that produces the same diagnostic for `-profile`. The printed text and code are the verified `CHECK` lines of `docs/generated/tests/design/cross-cutting/targets/profile-unknown-name-rejected.slang` (`unknown profile 'bogus_profile'`, `E00014`). | added a note that the `-capability` handler reuses the unknown-profile diagnostic |
| a36b7c8757cc | fixed | `source/slang/slang-profile.cpp:25-37` maps a profile version to `CapabilityName::<TAG>`; `source/slang/slang-capabilities.capdef:1792-1799` shows `alias sm_6_0_version` carrying `spirv_1_3` (reached from `sm_6_0` at `:1803` and from `DX_6_0` at `:2025`), and `:2185-2195` shows `alias GLSL_450` listing `spirv_1_3` directly, so a cross-family profile does supply a SPIR-V version atom. The no-profile case is the verified `SPIRV: Version: 1.5` line of `docs/generated/tests/design/cross-cutting/targets/spirv-direct-path-defaults-to-spirv-1-5.slang`. Re-confirmed against the compiler on this pass: `slangc -O0 -target spirv -entry main -stage compute t.slang -o t.spv` writes header version 1.5 with no `-profile`, and 1.3 under each of `-profile glsl_450`, `-profile sm_6_0`, `-profile spirv_1_3`. | distinguished "no `-profile`" from "cross-family `-profile`" and gave the emitted `; Version:` line for each |
| 97e3c6404b08 | fixed | `source/slang/slang-capabilities.capdef:1532` is `alias node = _node + _sm_6_8;`; `source/slang/slang-profile.cpp:38-47` adds `CapabilityName::node` to the profile's own capability set whenever the stage is `Stage::Node`, so the atom is in the promised set the check compares against rather than being checked against `-profile` | stated that the floor raises what a node entry point promises, which is why a below-`sm_6_8` node entry point is not diagnosed |
| d25f710d2ffa | rejected-out-of-scope | The document's `## What is not in this document` already delegates "the detailed profile-version table" to `source/slang/slang-profile-defs.h`, and the accepted `-profile` spellings are listed in `docs/command-line-slangc-reference.md` under `-profile`. | — |
| 70c758b923f2 | rejected-out-of-scope | The `-target` spellings are owned by `docs/command-line-slangc-reference.md` (the `target` list at its `#target-1` anchor, which the document already names as the owner of user-facing target documentation). The authoritative table is `s_compileTargetInfos` in `source/core/slang-type-text-util.cpp:47-108`. | — |
