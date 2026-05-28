---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-22T00:00:00+00:00
source_commit: 6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
watched_paths_digest: 95ca286185fd80c2dcfb776eb9b74e10e9d81f301216a41276a5d3f6a7328814
source_doc: docs/generated/design/cross-cutting/targets.md
source_doc_digest: c52732a0125ff8e2d8f308edaf694a2acd6b3671cb90433ebf99ed2d996ee3cd
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for cross-cutting/targets

## Intent
Tests verify the target-abstraction claims in
[`docs/generated/design/cross-cutting/targets.md`](../../../docs/generated/design/cross-cutting/targets.md):
the relationship between `TargetRequest`, `Profile`, and capability
sets; how target selection affects IR lowering and emit; capability-
mismatch diagnostics; the text/binary target table. The bundle is
multi-backend-heavy by design — most claims are statements about
cross-target behavior.

> **Recovery note.** This bundle was partially produced in wave 4 by
> a subagent that stalled while iterating on a single diagnostic test
> (`raytracing-capability-rejected-on-fragment.slang`). The 13 test
> files survived; the README.md and the one failing test's CHECK text
> were completed in the main loop. The stall surfaced one new
> framework lesson, captured below under `## Framework feedback for
> _common.md`.


## Functional coverage
| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A `[require(hlsl, glsl)]` conjunction populates the `target` keyhole with two different atoms; the doc says such conjunctions are incompatible and the compiler rejects with `E36111: disallowed capability`. | negative | [#capability-system](../../../docs/generated/design/cross-cutting/targets.md#capability-system) | [`capability-conflict-two-target-atoms-rejected.slang`](capability-conflict-two-target-atoms-rejected.slang) |
| A capability name expands to a disjunction of conjunctions (the doc's `raytracing` example). Using a raytracing-only intrinsic from a compute entry point on HLSL is rejected with a stage / capability mismatch diagnostic. | negative | [#capability-system](../../../docs/generated/design/cross-cutting/targets.md#capability-system) | [`raytracing-capability-rejected-on-fragment.slang`](raytracing-capability-rejected-on-fragment.slang) |
| The `raytracing` capability requires a raygen/closesthit/... stage atom; calling `TraceRay` from a `vertex` entry point populates the `stage` keyhole with `vertex`, conflicting with the raytracing-stage atoms and producing a stage-mismatch diagnostic. | negative | [#capability-system](../../../docs/generated/design/cross-cutting/targets.md#capability-system) | [`raytracing-intrinsic-on-vertex-stage-rejected.slang`](raytracing-intrinsic-on-vertex-stage-rejected.slang) |
| The capability system rejects use of a feature whose stage atom is incompatible with the active entry point's stage; `discard` (fragment-only) inside a compute entry point on HLSL is rejected. | negative | [#capability-system](../../../docs/generated/design/cross-cutting/targets.md#capability-system) | [`capability-mismatch-fragment-in-compute.slang`](capability-mismatch-fragment-in-compute.slang) |
| `WaveActiveSum` carries a wave-intrinsic capability available on the HLSL target but not on the C++ shader target; calling it from a C++ entry point trips the capability join and surfaces a target-mismatch diagnostic. | negative | [#capability-system](../../../docs/generated/design/cross-cutting/targets.md#capability-system) | [`wave-active-sum-on-cpp-rejected.slang`](wave-active-sum-on-cpp-rejected.slang) |
| A `__target_switch` where every text-emit target's case falls through to a single shared body still has its body selected by `slang-ir-specialize-target-switch.cpp` for each target. | boundary | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`target-switch-all-fallthrough.slang`](target-switch-all-fallthrough.slang) |
| A `__target_switch` whose only arm is `default:` provides a universal fallback; every target's `slang-ir-specialize-target-switch.cpp` lowering selects the default body. | boundary | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`target-switch-only-default-arm.slang`](target-switch-only-default-arm.slang) |
| A `__target_switch` with a single `case` arm: under the matching target the arm body is selected by `slang-ir-specialize-target-switch.cpp` — the minimum non-empty target-switch shape. | boundary | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`target-switch-single-arm.slang`](target-switch-single-arm.slang) |
| A `__target_switch` with no arm matching the active target and no `default:` is rejected with a stage / capability diagnostic when reached from an entry point on the unmatched target. | negative | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`target-switch-missing-arm-rejected.slang`](target-switch-missing-arm-rejected.slang) |
| A `__target_switch` with zero arms is accepted by the parser and dropped by `slang-ir-specialize-target-switch.cpp` — the degenerate lower edge of arm count. | boundary | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`target-switch-empty-body.slang`](target-switch-empty-body.slang) |
| Stress pattern: a `__target_switch` nested inside another `__target_switch`, both resolved by `slang-ir-specialize-target-switch.cpp` for each target — the IR pass handles recursive case-resolution. | stress | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`target-switch-nested-stress.slang`](target-switch-nested-stress.slang) |
| Target choice surfaces through target-specific lowering passes; a `[shader("compute")]` entry point emits the target-shaped compute marker on each text-emit backend. | functional | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`per-target-entry-point-marker.slang`](per-target-entry-point-marker.slang) |
| Target-specific lowering passes (slang-ir-hlsl-legalize, slang-ir-glsl-legalize, slang-ir-spirv-legalize, slang-ir-metal-legalize, slang-ir-wgsl-legalize) translate a generic resource binding into the target's native form. | functional | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`per-target-legalize-resource-binding.slang`](per-target-legalize-resource-binding.slang) |
| Target-specific lowering passes spell vector types in each language's idiom (HLSL `float3`, GLSL `vec3`, SPIR-V `OpTypeVector`, Metal `float3`, WGSL `vec3<f32>`, CUDA `float3`, CPP `Vector<float, 3>`). | functional | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`per-target-legalize-vector-type-shape.slang`](per-target-legalize-vector-type-shape.slang) |
| The CUDA target-specific lowering pass (`slang-ir-cuda-immutable-load.cpp`, named in the doc) rewrites reads from `uniform` globals as `__ldg(...)` calls on CUDA only; HLSL/GLSL/SPIR-V/Metal/WGSL/CPP read the field directly. | functional | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`cuda-immutable-load-uses-ldg.slang`](cuda-immutable-load-uses-ldg.slang) |
| When the active target has no matching `case` arm in `__target_switch` but a `default:` arm exists, `slang-ir-specialize-target-switch.cpp` selects the default — the legal alternative to the negative "no arm, no default" companion. | boundary | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`target-switch-missing-arm-falls-to-default.slang`](target-switch-missing-arm-falls-to-default.slang) |
| `slang-ir-specialize-target-switch.cpp` resolves `[target]` switches against the active TargetRequest; a `__target_switch` with per-target branches surfaces the selected branch's body in the target's emitted text. | functional | [#how-target-choice-affects-ir](../../../docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir) | [`target-switch-selects-active-branch.slang`](target-switch-selects-active-branch.slang) |
| A Profile binds a `Family` + `Version` to a target. `-profile sm_6_0` parses and the resulting capability set lets a compute shader compile on HLSL; `-profile glsl_450` does the same on GLSL. | functional | [#profiles](../../../docs/generated/design/cross-cutting/targets.md#profiles) | [`profile-binds-stage-and-family.slang`](profile-binds-stage-and-family.slang) |
| A Profile carries a Stage; `-profile cs_6_5` binds Compute. Asking for `-stage vertex` on the same command line populates the entry point's stage with two different atoms and produces `E00031: conflicting stages`. | negative | [#profiles](../../../docs/generated/design/cross-cutting/targets.md#profiles) | [`profile-stage-conflict-with-explicit-stage-rejected.slang`](profile-stage-conflict-with-explicit-stage-rejected.slang) |
| A `-profile` value that does not appear in slang-profile-defs.h is rejected with `E00014: unknown profile`; profile parsing is closed-set against the Family + Version table. | negative | [#profiles](../../../docs/generated/design/cross-cutting/targets.md#profiles) | [`profile-unknown-name-rejected.slang`](profile-unknown-name-rejected.slang) |
| The doc's Profiles section gives `-target spirv -profile glsl_450` as an example that selects a profile-driven capability set. A profile that pairs SPIR-V with GLSL 450 parses and compiles a vanilla compute shader. | functional | [#profiles](../../../docs/generated/design/cross-cutting/targets.md#profiles) | [`profile-spirv-1-5-compiles.slang`](profile-spirv-1-5-compiles.slang) |
| The highest documented GLSL profile (`glsl_460` — top of the GLSL family in slang-profile-defs.h) compiles a vanilla compute shader; the emitted module declares `#version 460`. | boundary | [#profiles](../../../docs/generated/design/cross-cutting/targets.md#profiles) | [`profile-edge-glsl-460.slang`](profile-edge-glsl-460.slang) |
| The highest documented HLSL Shader Model profile (`sm_6_9` per DX_6_9 in slang-profile-defs.h) compiles a vanilla compute shader; the upper-edge Profile value parses and selects a valid capability set. | boundary | [#profiles](../../../docs/generated/design/cross-cutting/targets.md#profiles) | [`profile-edge-highest-sm-6-9.slang`](profile-edge-highest-sm-6-9.slang) |
| The highest documented SPIR-V profile (`spirv_1_6` — top of the SPIRV family in slang-profile-defs.h) compiles a vanilla compute shader; the emitted module declares `Version: 1.6`. | boundary | [#profiles](../../../docs/generated/design/cross-cutting/targets.md#profiles) | [`profile-edge-spirv-1-6.slang`](profile-edge-spirv-1-6.slang) |
| The lowest documented HLSL profile (`sm_4_0` — the floor of the DX_4_0 family in slang-profile-defs.h) compiles a vanilla compute shader; the profile-driven capability set is satisfied by the trivial kernel. | boundary | [#profiles](../../../docs/generated/design/cross-cutting/targets.md#profiles) | [`profile-edge-lowest-sm-4-0.slang`](profile-edge-lowest-sm-4-0.slang) |
| The lowest documented SPIR-V profile (`spirv_1_0` — floor of the SPIRV family in slang-profile-defs.h) compiles a vanilla compute shader; the emitted module declares `Version: 1.0`. | boundary | [#profiles](../../../docs/generated/design/cross-cutting/targets.md#profiles) | [`profile-edge-spirv-1-0.slang`](profile-edge-spirv-1-0.slang) |
| Capability sets are joined against the active profile. `-restrictive-capability-check` upgrades a missing-atom mismatch to an error; without the flag the same input emits an implicit-upgrade warning. | negative | [#runtime-representation](../../../docs/generated/design/cross-cutting/targets.md#runtime-representation) | [`restrictive-capability-check-rejects-missing-atom.slang`](restrictive-capability-check-rejects-missing-atom.slang) |
| Using `RayQuery` (an `sm_6_3` feature) in an entry point with `-profile sm_6_0` under `-restrictive-capability-check` reports the Profile's Version is below the required capability and rejects with `E41013`. | negative | [#runtime-representation](../../../docs/generated/design/cross-cutting/targets.md#runtime-representation) | [`sm-6-3-feature-on-sm-6-0-rejected-restrictive.slang`](sm-6-3-feature-on-sm-6-0-rejected-restrictive.slang) |
| A `-target` value that does not appear in the documented target table is rejected with `E00013: unknown code generation target`; target parsing is closed-set against `SlangCompileTarget`. | negative | [#targets](../../../docs/generated/design/cross-cutting/targets.md#targets) | [`unknown-target-name-rejected.slang`](unknown-target-name-rejected.slang) |
| Stress probe of the Targets table: the same dense compute kernel (scalar, vector, and matrix work; multiple buffer parameters) compiles to all 7 text-emit targets — HLSL, GLSL, SPIR-V asm, Metal, WGSL, CUDA, C++. | stress | [#targets](../../../docs/generated/design/cross-cutting/targets.md#targets) | [`all-seven-text-targets-dense-stress.slang`](all-seven-text-targets-dense-stress.slang) |
| The Targets table lists `SLANG_DXIL` under the HLSL emit backend with DXIL produced downstream by DXC; `-target dxil -profile cs_6_0` emits a DXIL module whose canonical entry-point declaration `define void @main()` is present in the assembly text. | expansion | [#targets](../../../docs/generated/design/cross-cutting/targets.md#targets) | [`dxil-binary-emission.slang`](dxil-binary-emission.slang) |
| The Targets table lists `SLANG_METAL_LIB` under the Metal Shading Language emit backend with the Metal library produced downstream by the Apple `metal` toolchain; `-target metallib` emits a Metal library whose canonical entry-point declaration `define void @main` appears in the assembly text. | expansion | [#targets](../../../docs/generated/design/cross-cutting/targets.md#targets) | [`metallib-binary-emission.slang`](metallib-binary-emission.slang) |
| The Targets table lists `SLANG_SPIRV` and `SLANG_SPIRV_ASM` as variants of the same emit backend; both `-target spirv-asm` produces SPIR-V text (the assembly-text-form observable in the no-GPU runner). | functional | [#targets](../../../docs/generated/design/cross-cutting/targets.md#targets) | [`spirv-asm-and-binary-target-variants.slang`](spirv-asm-and-binary-target-variants.slang) |
| The doc's Targets table lists HLSL/GLSL/SPIR-V/Metal/WGSL/CUDA/C++ as distinct emit backends; the same source compiles successfully to each text-emit target. | functional | [#targets](../../../docs/generated/design/cross-cutting/targets.md#targets) | [`target-table-all-text-emit.slang`](target-table-all-text-emit.slang) |
| A stage atom (`compute`) populates the `stage` keyhole; a target atom (`hlsl`, `glsl`, `spirv`, ...) populates the `target` keyhole. Because they populate different keyholes a `compute` entry point is compatible with every text-emit target. | functional | [#vocabulary](../../../docs/generated/design/cross-cutting/targets.md#vocabulary) | [`capability-conjunction-stage-and-target.slang`](capability-conjunction-stage-and-target.slang) |
| The stage / target keyhole independence claim extends to `cuda`: a `compute` entry point conjoined with the `cuda` target atom is a valid combination (different keyholes), mirroring the C-01 smoke probe at a single specific target. | boundary | [#vocabulary](../../../docs/generated/design/cross-cutting/targets.md#vocabulary) | [`capability-stage-and-target-conjunction-cuda.slang`](capability-stage-and-target-conjunction-cuda.slang) |


## Untested claims
| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| WebGPU runtime tests. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| LLVM-native binary emission. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| Slang round-trip (`-target slang`). | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| VM byte-code target (`-target hostvm`). | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| Any test that needs runtime device enumeration. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| Torch glue / `slang-python` bindings. | (unclassified) | [#slang-python](../../../docs/generated/design/cross-cutting/targets.md#slang-python) | Reason and explanation to be refined by the next regeneration. |


## Doc gaps observed
| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#targets](../../../docs/generated/design/cross-cutting/targets.md#targets) | undocumented-behavior | `## Targets` enumerates DXIL/MSL-binary/WebGPU-binary as binary output formats but does not state which combinations of `-target` + `-profile` produce them via slangc CLI vs. via the runtime API. |  |
| [#profiles](../../../docs/generated/design/cross-cutting/targets.md#profiles) | undocumented-behavior | `## Profiles` does not enumerate the available profile names; tests had to discover spellings from `tests/` and `source/slang/slang-profile-defs.h`. |  |
| [#capability-system](../../../docs/generated/design/cross-cutting/targets.md#capability-system) | undocumented-behavior | `## Capability system` describes conjunction-of-disjunctions semantics with the `raytracing` example, but does not enumerate which capability atoms are stage-only vs. target-only vs. both. |  |
| [#capabilityquery](../../../docs/generated/design/cross-cutting/targets.md#capabilityquery) | undocumented-behavior | No documented user-facing capability-introspection (e.g. a `__capability_query` builtin) — observed via diagnostic emission only. |  |
| (unspecified) | undocumented-behavior | The DXIL binary target is not exercised here; the doc lists it but binary emission requires a downstream-compiler binary that the agentic runner does not assume. |  |
| [#targetswitch](../../../docs/generated/design/cross-cutting/targets.md#targetswitch) | undocumented-behavior | `__target_switch` is named in the IR section but not documented user-side: its arm-count edges (0 arms, 1 arm, all-fallthrough, `default:` fallback) had to be discovered empirically. | A short user-side spec would let tests anchor to a precise claim instead of the specialization pass's name. |
| [#version](../../../docs/generated/design/cross-cutting/targets.md#version) | ambiguous-claim | `## Profiles` mentions Family + Version + Stage but does not state what happens when a sub-version request (e.g. `glsl_150`) is below the back-end's floor; the back-end silently clamps to `#version 450`. Documented clamping rules would make profile-edge tests unambiguous. |  |
| [#capability-system](../../../docs/generated/design/cross-cutting/targets.md#capability-system) | undocumented-behavior | The `-restrictive-capability-check` flag is exercised by tests but not named in `## Capability system`; only the "Computing the join" bullet under Runtime representation hints at its role. |  |
| (unspecified) | undocumented-behavior | Per-target stage rejection (e.g. fragment-on-cuda) is not robust: one observed shape produced a SIGSEGV rather than a clean diagnostic. | The doc should state which stage × target pairs are rejected at check time vs. accepted vs. unsupported. |


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
