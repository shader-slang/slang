---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T18:30:00+00:00
source_commit: 1e0d460c1cb410005c4f775ba11fbc803cc8c16d
watched_paths_digest: 95ca286185fd80c2dcfb776eb9b74e10e9d81f301216a41276a5d3f6a7328814
source_doc: docs/llm-generated/cross-cutting/targets.md
source_doc_digest: c52732a0125ff8e2d8f308edaf694a2acd6b3671cb90433ebf99ed2d996ee3cd
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for cross-cutting/targets

## Intent

Tests verify the target-abstraction claims in
[`docs/llm-generated/cross-cutting/targets.md`](../../../docs/llm-generated/cross-cutting/targets.md):
the relationship between `TargetRequest`, `Profile`, and capability
sets; how target selection affects IR lowering and emit; capability-
mismatch diagnostics; the text/binary target table. The bundle is
multi-backend-heavy by design — most claims are statements about
cross-target behavior.

> **Recovery note.** This bundle was partially produced in wave 4 by
> a subagent that stalled while iterating on a single diagnostic test
> (`raytracing-capability-rejected-on-fragment.slang`). The 13 test
> files survived; the BUNDLE.md and the one failing test's CHECK text
> were completed in the main loop. The stall surfaced one new
> framework lesson, captured below under `## Framework feedback for
> _common.md`.

## Claims enumerated

| Claim ID | Anchor                          | Claim (one line)                                                                                                              | Tests                                                                                                                                                  |
| -------- | ------------------------------- | ----------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| C-01     | `#vocabulary`                   | A capability alias expands to a disjunction of conjunctions; conjoining stage + target atoms is satisfied by matching builds. | `capability-conjunction-stage-and-target.slang`                                                                                                        |
| C-02     | `#capability-system`            | Using a target-only feature in a non-matching profile is rejected with a capability-mismatch diagnostic.                      | `capability-mismatch-fragment-in-compute.slang`                                                                                                        |
| C-03     | `#capability-system`            | Using a stage-restricted intrinsic from a non-matching stage is rejected with a stage-mismatch diagnostic.                    | `raytracing-capability-rejected-on-fragment.slang`                                                                                                     |
| C-04     | `#how-target-choice-affects-ir` | Selecting `-target cuda` lowers immutable global reads to `__ldg(...)`.                                                       | `cuda-immutable-load-uses-ldg.slang`                                                                                                                   |
| C-05     | `#how-target-choice-affects-ir` | The entry-point marker differs per target (HLSL `[shader(...)]` vs. SPIR-V `OpEntryPoint` vs. Metal `[[kernel]]`).             | `per-target-entry-point-marker.slang`                                                                                                                  |
| C-06     | `#how-target-choice-affects-ir` | Resource bindings are legalized per target (HLSL `register(u0)` vs. SPIR-V `Binding 0`).                                      | `per-target-legalize-resource-binding.slang`                                                                                                           |
| C-07     | `#how-target-choice-affects-ir` | Vector types are legalized to per-target spellings (`float3` vs. `vec3` vs. SPIR-V composite).                                | `per-target-legalize-vector-type-shape.slang`                                                                                                          |
| C-08     | `#how-target-choice-affects-ir` | A `#if SLANG_*` (or target-conditional construct) selects the active branch per `-target`.                                    | `target-switch-selects-active-branch.slang`                                                                                                            |
| C-09     | `#profiles`                     | Profile binds stage and family (`cs_6_5` → compute / SM 6.5).                                                                 | `profile-binds-stage-and-family.slang`                                                                                                                 |
| C-10     | `#profiles`                     | A SPIR-V profile (`-profile spirv_1_5`) compiles cleanly when capabilities match.                                             | `profile-spirv-1-5-compiles.slang`                                                                                                                     |
| C-11     | `#runtime-representation`       | A `__target_switch` / restrictive capability check rejects a missing atom.                                                    | `restrictive-capability-check-rejects-missing-atom.slang`                                                                                              |
| C-12     | `#targets`                      | `-target spirv-asm` (text) and `-target spirv` (binary) both compile.                                                         | `spirv-asm-and-binary-target-variants.slang`                                                                                                           |
| C-13     | `#targets`                      | All documented text-emit targets accept a minimal compute kernel.                                                             | `target-table-all-text-emit.slang`                                                                                                                     |

## Tests in this bundle

| File                                                       | Intent     | Doc anchor                       |
| ---------------------------------------------------------- | ---------- | -------------------------------- |
| `capability-conjunction-stage-and-target.slang`            | functional | `#vocabulary`                    |
| `capability-mismatch-fragment-in-compute.slang`            | negative   | `#capability-system`             |
| `cuda-immutable-load-uses-ldg.slang`                       | functional | `#how-target-choice-affects-ir`  |
| `per-target-entry-point-marker.slang`                      | functional | `#how-target-choice-affects-ir`  |
| `per-target-legalize-resource-binding.slang`               | functional | `#how-target-choice-affects-ir`  |
| `per-target-legalize-vector-type-shape.slang`              | functional | `#how-target-choice-affects-ir`  |
| `profile-binds-stage-and-family.slang`                     | functional | `#profiles`                      |
| `profile-spirv-1-5-compiles.slang`                         | functional | `#profiles`                      |
| `raytracing-capability-rejected-on-fragment.slang`         | negative   | `#capability-system`             |
| `restrictive-capability-check-rejects-missing-atom.slang`  | negative   | `#runtime-representation`        |
| `spirv-asm-and-binary-target-variants.slang`               | functional | `#targets`                       |
| `target-switch-selects-active-branch.slang`                | functional | `#how-target-choice-affects-ir`  |
| `target-table-all-text-emit.slang`                         | functional | `#targets`                       |
| `profile-edge-lowest-sm-4-0.slang`                         | boundary   | `#profiles`                      |
| `profile-edge-highest-sm-6-9.slang`                        | boundary   | `#profiles`                      |
| `profile-edge-spirv-1-0.slang`                             | boundary   | `#profiles`                      |
| `profile-edge-spirv-1-6.slang`                             | boundary   | `#profiles`                      |
| `profile-edge-glsl-460.slang`                              | boundary   | `#profiles`                      |
| `profile-unknown-name-rejected.slang`                      | negative   | `#profiles`                      |
| `profile-stage-conflict-with-explicit-stage-rejected.slang`| negative   | `#profiles`                      |
| `target-switch-single-arm.slang`                           | boundary   | `#how-target-choice-affects-ir`  |
| `target-switch-only-default-arm.slang`                     | boundary   | `#how-target-choice-affects-ir`  |
| `target-switch-all-fallthrough.slang`                      | boundary   | `#how-target-choice-affects-ir`  |
| `target-switch-empty-body.slang`                           | boundary   | `#how-target-choice-affects-ir`  |
| `target-switch-missing-arm-falls-to-default.slang`         | boundary   | `#how-target-choice-affects-ir`  |
| `target-switch-missing-arm-rejected.slang`                 | negative   | `#how-target-choice-affects-ir`  |
| `target-switch-nested-stress.slang`                        | stress     | `#how-target-choice-affects-ir`  |
| `capability-stage-and-target-conjunction-cuda.slang`       | boundary   | `#vocabulary`                    |
| `capability-conflict-two-target-atoms-rejected.slang`      | negative   | `#capability-system`             |
| `raytracing-intrinsic-on-vertex-stage-rejected.slang`      | negative   | `#capability-system`             |
| `sm-6-3-feature-on-sm-6-0-rejected-restrictive.slang`      | negative   | `#runtime-representation`        |
| `wave-active-sum-on-cpp-rejected.slang`                    | negative   | `#capability-system`             |
| `unknown-target-name-rejected.slang`                       | negative   | `#targets`                       |
| `all-seven-text-targets-dense-stress.slang`                | stress     | `#targets`                       |

## Doc gaps observed

- `## Targets` enumerates DXIL/MSL-binary/WebGPU-binary as binary
  output formats but does not state which combinations of `-target`
  + `-profile` produce them via slangc CLI vs. via the runtime API.
- `## Profiles` does not enumerate the available profile names; tests
  had to discover spellings from `tests/` and `source/slang/slang-profile-defs.h`.
- `## Capability system` describes conjunction-of-disjunctions
  semantics with the `raytracing` example, but does not enumerate
  which capability atoms are stage-only vs. target-only vs. both.
- No documented user-facing capability-introspection (e.g. a
  `__capability_query` builtin) — observed via diagnostic emission
  only.
- The DXIL binary target is not exercised here; the doc lists it but
  binary emission requires a downstream-compiler binary that the
  agentic runner does not assume.
- `__target_switch` is named in the IR section but not documented
  user-side: its arm-count edges (0 arms, 1 arm, all-fallthrough,
  `default:` fallback) had to be discovered empirically. A short
  user-side spec would let tests anchor to a precise claim instead
  of the specialization pass's name.
- `## Profiles` mentions Family + Version + Stage but does not state
  what happens when a sub-version request (e.g. `glsl_150`) is below
  the back-end's floor; the back-end silently clamps to `#version 450`.
  Documented clamping rules would make profile-edge tests
  unambiguous.
- The `-restrictive-capability-check` flag is exercised by tests but
  not named in `## Capability system`; only the "Computing the join"
  bullet under Runtime representation hints at its role.
- Per-target stage rejection (e.g. fragment-on-cuda) is not robust:
  one observed shape produced a SIGSEGV rather than a clean
  diagnostic. The doc should state which stage × target pairs are
  rejected at check time vs. accepted vs. unsupported.

## Out of scope (no-GPU runner)

- DXIL binary emission (needs `dxc.exe`).
- MSL binary emission (needs Apple toolchain).
- WebGPU runtime tests.
- LLVM-native binary emission.
- Slang round-trip (`-target slang`).
- VM byte-code target (`-target hostvm`).
- Torch glue / `slang-python` bindings.
- Any test that needs runtime device enumeration.

## Framework feedback for \_common.md

A diagnostic test whose CHECK text mismatches the actual compiler
message by a single token (e.g. `entry point` vs. `entrypoint`) can
cause the runner to drop into an iterative "find the right CHECK"
loop that confuses agents — the failure looks like "the diagnostic
chain is wrong" when in fact a single word in the expected text needs
fixing. Mitigation: always copy the diagnostic text **verbatim** from
slang-test's "Suggested annotations" output. The lesson is already
implicit in `_common.md`'s `DIAGNOSTIC_TEST` section but worth a
sharper "copy verbatim, do not paraphrase" note.
