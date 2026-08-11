---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:20:02Z
target_doc: cross-cutting/targets.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 12
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 3
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for cross-cutting/targets.md

## Summary

Twelve gaps were acted on: eight fixed, three rejected as
out-of-scope, one deferred; nothing was escalated as a compiler
defect. Both `ambiguous-claim` gaps turned out to be resolvable from
the watched `.capdef` and `slang-profile.cpp` — the `spirv_1_5`
fallback and the work-graph `_sm_6_8` floor are stated correctly by
the document but for non-obvious reasons, and both now say why the
observed behaviour follows — so neither became a finding. The three
rejections are the `-target` spelling list, the `-profile` name list,
and the per-family "where does the version appear in the emitted
text" question; the first two are owned by
`docs/command-line-slangc-reference.md`, and the third is exactly the
"on target T, construct X is emitted as Y" form this page hands to
`docs/generated/design/target-pipelines/`. Two operator items follow
from this pass. First, the document was already at 23646 bytes
against its 24576-byte `size_cap_bytes` and is now 26890 (lint
reports this as a warning, not an error); every added sentence was
requested by a gap and is source-confirmed, so the cap should be
raised to roughly 28672 or `## Per-target pass pipelines` /
`## Adding a new target` split into a peer page. Second, three fixes
and the deferral rest on files the manifest does not watch —
`source/slang/slang-compiler.h`,
`source/slang/slang-options.cpp`, and
`source/slang/slang-target.cpp` (the last already cited by the
document) — and adding those three to `watched_paths` would also
unblock `ba43401fe07f`. Gap `34b5fafdf292` was documented rather
than escalated because the diagnostic reuse follows from the shared
`-profile foo+cap` spelling; if the operator wants the misleading
message tracked, it is a diagnostics-wording finding on the tests
side, not a documentation defect.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 4ca278c8d71b | fixed | `source/slang/hlsl.meta.slang:14130-14132` declares `[require(abort)] void abort<each T>(NativeString format, expand each T args);`; `source/slang/slang-capabilities.capdef:2476` is `alias abort = GL_EXT_shader_abort;` | named the `abort` builtin and its call form in the `[Compound]` paragraph |
| c15c7c4c2e96 | fixed | `source/slang/hlsl.meta.slang:22376, 22404, 22630, 22655` (sphere) and `22432, 22479, 22526, 22552, 22578, 22604` (LSS), each preceded by `[require(glsl_hlsl_spirv, rayquery_*_nv)]` and `[__requiresNVAPI]` | listed the ten gated `RayQuery` methods and the NVAPI requirement, in the same `[Compound]` edit as 4ca278c8d71b |
| c0d54084a05b | fixed | `source/slang/slang-compiler.h:239-244` (`maybeDiagnose` drops `Capability`-category diagnostics under `IgnoreCapabilities`) and `:255-264` (`maybeDiagnoseWarningOrError` picks the error form under `RestrictiveCapabilityCheck`); the two printed forms are the verified `CHECK-ERR` / `CHECK-WARN` lines of `docs/generated/tests/design/cross-cutting/targets/restrictive-capability-check-rejects-missing-atom.slang` | added a paragraph under `### Runtime representation` naming both flags and the diagnostic each selects |
| a2ab094d2c29 | fixed | `source/slang/slang-capabilities.capdef:683` declares `def SPV_KHR_cooperative_matrix : _spirv_1_6 + SPV_EXT_physical_storage_buffer + SPV_KHR_vulkan_memory_model;`; the emitted rendering is the verified `SPIRV-DAG` lines of `docs/generated/tests/design/cross-cutting/targets/capability-extension-inherits-version-floor.slang` | added the `-target spirv-asm -capability SPV_KHR_cooperative_matrix` worked example to the version-floor paragraph |
| a9fa50665328 | rejected-out-of-scope | The document's `## What is not in this document` bullet assigns "how a particular construct is spelled in the emitted language" to `docs/generated/design/target-pipelines/`, adding "even when the reason is a capability". A per-family statement of where a version word appears in emitted SPIR-V / GLSL / HLSL / MSL is that form. | — |
| c402197b09a5 | fixed | `source/slang/core.meta.slang:1076-1087` is a `__target_switch` with a `case hlsl:` arm and a `default:` arm; fall-through and the no-arm outcome are the verified `CHECK` lines of `docs/generated/tests/design/cross-cutting/targets/target-switch-missing-arm-falls-to-default.slang` and `target-switch-missing-arm-rejected.slang` (the entry point, not the switch, is rejected) | added a minimal `__target_switch` example and the no-matching-arm rule under **Specialization passes** |
| ba43401fe07f | deferred | Blocked twice over: the positive triple's diagnostic text can only be settled by running `slangc`, which this host cannot do (the tree's build is Linux x86-64, the host is arm64); and the bundle README records that the compiler binary the tests ran against predates `conflicting-explicit-capability-and-profile`, so `capability-added-to-cross-family-profile-not-diagnosed.slang` passing is not evidence for the carve-out branch either. The call site is `source/slang/slang-options.cpp:4508-4538`, outside `watched_paths`. Follow-up: re-run once a host-native build exists. | — |
| 34b5fafdf292 | fixed | `source/slang/slang-options.cpp:3306-3312` reports `Diagnostics::UnknownProfile` for a `-capability` atom that `findCapabilityName` rejects; `:2671-2677` is the `Profile::lookUp` path that produces the same diagnostic for `-profile`. The printed text and code are the verified `CHECK` lines of `docs/generated/tests/design/cross-cutting/targets/profile-unknown-name-rejected.slang` (`unknown profile 'bogus_profile'`, `E00014`). | added a note that the `-capability` handler reuses the unknown-profile diagnostic |
| a36b7c8757cc | fixed | `source/slang/slang-profile.cpp:25-37` maps a profile version to `CapabilityName::<TAG>`; `source/slang/slang-capabilities.capdef:1792-1799` shows `alias sm_6_0_version` carrying `spirv_1_3` (reached from `sm_6_0` at `:1803` and from `DX_6_0` at `:2025`), and `:2185-2195` shows `alias GLSL_450` listing `spirv_1_3` directly, so a cross-family profile does supply a SPIR-V version atom. The no-profile case is the verified `SPIRV: Version: 1.5` line of `docs/generated/tests/design/cross-cutting/targets/spirv-direct-path-defaults-to-spirv-1-5.slang`. | distinguished "no `-profile`" from "cross-family `-profile`" and gave the emitted `; Version:` line for each |
| 97e3c6404b08 | fixed | `source/slang/slang-capabilities.capdef:1532` is `alias node = _node + _sm_6_8;`; `source/slang/slang-profile.cpp:38-47` adds `CapabilityName::node` to the profile's own capability set whenever the stage is `Stage::Node`, so the atom is in the promised set the check compares against rather than being checked against `-profile` | stated that the floor raises what a node entry point promises, which is why a below-`sm_6_8` node entry point is not diagnosed |
| d25f710d2ffa | rejected-out-of-scope | The document's `## What is not in this document` already delegates "the detailed profile-version table" to `source/slang/slang-profile-defs.h`, and the accepted `-profile` spellings are listed in `docs/command-line-slangc-reference.md` under `-profile`. The macro table itself is outside `watched_paths`. | — |
| 70c758b923f2 | rejected-out-of-scope | The `-target` spellings are owned by `docs/command-line-slangc-reference.md` (the `target` list at its `#target-1` anchor, which the document already names as the owner of user-facing target documentation). The authoritative table is `s_compileTargetInfos` in `source/core/slang-type-text-util.cpp:47-108`, outside `watched_paths`. | — |
