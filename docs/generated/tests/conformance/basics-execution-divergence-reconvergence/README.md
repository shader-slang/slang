---
generated: true
model: claude-sonnet-4-6
generated_at: 2026-06-02T07:03:42+00:00
source_commit: 9c1a6a00ef413932805da5b813465a7a9d517fb9
watched_paths_digest: f85ba3748551a080ccaa6925f67b24f7c66cc9a697da96cd9e36f0c365c0369b
source_doc: docs/language-reference/basics-execution-divergence-reconvergence.md
source_doc_digest: 5390dfefbb80dbe1826fa029f9aa83391e6d5018a0017c1187364e7d3ae0f1df
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for conformance/basics-execution-divergence-reconvergence

## Intent

Tests verify divergence and reconvergence claims in the **language reference** at
[`docs/language-reference/basics-execution-divergence-reconvergence.md`](../../../../language-reference/basics-execution-divergence-reconvergence.md).
The doc describes how execution diverges and reconverges in structured control flow (`if`, `switch`, `loop`)
and how wave-tangled functions behave on divergent paths.

Because wave/subgroup runtime value behavior is only observable on a GPU, the coverage strategy is
**emission-first**: SPIRV, HLSL, and GLSL emission tests pin the structural opcodes and intrinsics the
doc commits to (`OpSelectionMerge`, `OpBranchConditional`, `OpSwitch`, `OpLoopMerge`,
`GroupNonUniform*`, `WaveActiveMin/Max`, `subgroupMin/Max`). GPU-value claims are recorded in
`## Untested claims` with reason `gpu-other`.

## Claims

### Preamble (lines 1–31)

- **C1**: Threads are on a _uniform/converged path_ when their execution has not diverged or has reconverged; control flow on a uniform path is called _uniform_.
- **C2**: Two convergence scopes exist: _thread-group-uniform path_ (all threads in the thread group) and _wave-uniform path_ (all threads in the wave).
- **C3**: A _mutually convergent_ set refers to threads in a wave that are on a mutually uniform path; when execution has diverged there is more than one such set.

### Divergence and Reconvergence in Structured Control Flow (`#divergence`)

- **C4**: In an `if` statement (with `then` and `else`), divergence occurs when some threads take the `then` branch and others take the `else` branch; reconvergence occurs when threads exit both branches.
- **C5**: In an `if` statement without an `else`, reconvergence still occurs at the point after the `if` statement.
- **C6**: In a `switch` statement, divergence occurs when threads jump to different case groups; reconvergence occurs when threads exit the switch statement.
- **C7**: In a `switch` statement, reconvergence between threads on adjacent case label groups occurs on a fall-through from one case to the next.
- **C8**: In a loop statement, divergence occurs when some threads exit the loop while others continue; reconvergence occurs when all threads have exited the loop.

### Thread-Group-Tangled Functions on Divergent Paths

- **C9**: Thread-group-tangled functions are supported only on thread-group-uniform paths; invoking them on a divergent path is undefined behavior.

### Wave-Tangled Functions on Divergent Paths

- **C10**: Not all targets support wave-tangled functions on divergent paths; when unsupported, results are undefined.
- **C11**: When supported, wave-tangled functions on divergent paths apply only between the mutually convergent thread set (synchronization occurs only between threads on the same path).

## Functional coverage

| Claim                                                                                                                                                                                         | Intent     | Anchor                                                                                                | Tests                                                                                                |
| --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | ----------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| C4: In an `if` statement (then+else), SPIRV emits `OpSelectionMerge` + `OpBranchConditional` marking the divergence point and merge block.                                                    | functional | [#divergence](../../../../language-reference/basics-execution-divergence-reconvergence.md#divergence) | [`if-divergence-reconvergence-emission.slang`](if-divergence-reconvergence-emission.slang)           |
| C5: In an `if` without `else`, SPIRV still emits `OpSelectionMerge` + `OpBranchConditional`; reconvergence at the single merge block after the if.                                            | boundary   | [#divergence](../../../../language-reference/basics-execution-divergence-reconvergence.md#divergence) | [`if-no-else-reconvergence-emission.slang`](if-no-else-reconvergence-emission.slang)                 |
| C6: In a `switch` statement, SPIRV emits `OpSelectionMerge` + `OpSwitch` to encode divergence into case groups and reconvergence at the merge block.                                          | functional | [#divergence](../../../../language-reference/basics-execution-divergence-reconvergence.md#divergence) | [`switch-divergence-reconvergence-emission.slang`](switch-divergence-reconvergence-emission.slang)   |
| C7: Switch fall-through: SPIRV emits a plain `OpBranch` (not a new `OpSelectionMerge`) between adjacent case groups; the outer `OpSwitch` dispatches all cases to a single merge block.       | boundary   | [#divergence](../../../../language-reference/basics-execution-divergence-reconvergence.md#divergence) | [`switch-fallthrough-reconvergence-emission.slang`](switch-fallthrough-reconvergence-emission.slang) |
| C8: In a loop, SPIRV emits `OpLoopMerge` marking the block where all threads reconverge after exiting the loop.                                                                               | functional | [#divergence](../../../../language-reference/basics-execution-divergence-reconvergence.md#divergence) | [`loop-divergence-reconvergence-emission.slang`](loop-divergence-reconvergence-emission.slang)       |
| C11: Wave-tangled ops (`WaveActiveMin`/`WaveActiveMax`) inside a divergent `if` emit as `GroupNonUniform` ops in separate SPIRV branch arms, applying to each mutually-convergent thread set. | functional | [#divergence](../../../../language-reference/basics-execution-divergence-reconvergence.md#divergence) | [`wave-op-divergent-if-spirv-emission.slang`](wave-op-divergent-if-spirv-emission.slang)             |
| C11 (HLSL): `WaveActiveMin`/`WaveActiveMax` inside a divergent `if` emit as HLSL `WaveActiveMin`/`WaveActiveMax` intrinsics inside the respective if-arms.                                    | functional | [#divergence](../../../../language-reference/basics-execution-divergence-reconvergence.md#divergence) | [`wave-op-divergent-if-hlsl-emission.slang`](wave-op-divergent-if-hlsl-emission.slang)               |
| C11 (GLSL): `WaveActiveMin`/`WaveActiveMax` inside a divergent `if` emit as GLSL `subgroupMin`/`subgroupMax` intrinsics requiring `GL_KHR_shader_subgroup_arithmetic`.                        | functional | [#divergence](../../../../language-reference/basics-execution-divergence-reconvergence.md#divergence) | [`wave-op-divergent-if-glsl-emission.slang`](wave-op-divergent-if-glsl-emission.slang)               |

## Untested claims

| Claim                                                                                                                             | Reason         | Anchor                                                                                                                            | Why untested                                                                                                                                                                                                                                                                                                   |
| --------------------------------------------------------------------------------------------------------------------------------- | -------------- | --------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C1: Threads are on a uniform/converged path when execution has not diverged or has reconverged.                                   | (unclassified) | [#execution-divergence-and-reconvergence](../../../../language-reference/basics-execution-divergence-reconvergence.md)            | Definitional; no user-visible compiler behavior to test independently of C4–C8.                                                                                                                                                                                                                                |
| C2: Two convergence scopes — thread-group-uniform and wave-uniform.                                                               | (unclassified) | [#execution-divergence-and-reconvergence](../../../../language-reference/basics-execution-divergence-reconvergence.md)            | Definitional taxonomy with no independent observable surface.                                                                                                                                                                                                                                                  |
| C3: A mutually convergent set refers to threads in a wave on a mutually uniform path.                                             | (unclassified) | [#execution-divergence-and-reconvergence](../../../../language-reference/basics-execution-divergence-reconvergence.md)            | Definitional; observable only indirectly via C11's GPU value behavior.                                                                                                                                                                                                                                         |
| C9: Thread-group-tangled functions (e.g., `GroupMemoryBarrier`) on divergent paths are undefined behavior.                        | gpu-other      | [#thread-group-tangled-functions-on-divergent-paths](../../../../language-reference/basics-execution-divergence-reconvergence.md) | Undefined behavior has no observable slang-test-detectable surface; no diagnostic is specified. A GPU-runtime test could check that barriers on divergent paths don't deadlock in CI, but the outcome is undefined and CI validation would not be stable.                                                      |
| C10: Not all targets support wave-tangled functions on divergent paths; when unsupported results are undefined.                   | gpu-other      | [#wave-tangled-functions-on-divergent-paths](../../../../language-reference/basics-execution-divergence-reconvergence.md)         | Per-target support is listed in target-compatibility.md, not this doc. Verifying that a target silently produces undefined values requires a GPU runner and target-specific capability queries not available in slang-test.                                                                                    |
| C11 (GPU values): Even threads seeing `WaveActiveMin({0,2})=0`; odd threads seeing `WaveActiveMax({1,3})=3` after a divergent if. | gpu-other      | [#wave-tangled-functions-on-divergent-paths](../../../../language-reference/basics-execution-divergence-reconvergence.md)         | Requires a GPU runner with working subgroup support. A `COMPARE_COMPUTE -vk` test was authored but fails on the local macOS/MoltenVK environment due to descriptor buffer allocation errors in the test harness (JSON RPC failure), so it was not committed. CI nightly with a discrete GPU would validate it. |

## Doc gaps observed

| Anchor                                                                                                                                                              | Kind            | Gap                                                                                                                                                                                                                                                    | Suggested addition                                                                                                                                                                              |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#wave-tangled-functions-on-divergent-paths](../../../../language-reference/basics-execution-divergence-reconvergence.md#wave-tangled-functions-on-divergent-paths) | missing-surface | The doc says "see target platforms for details" about which targets support wave-tangled functions on divergent paths, but does not name a Slang capability query or attribute that a shader author can use to branch at compile time on this feature. | Add a note naming the capability flag or `__require_feature` call a shader can use, or link to the specific section of target-compatibility.md that enumerates which targets have this support. |
