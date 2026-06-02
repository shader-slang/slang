---
generated: true
model: claude-sonnet-4-6
generated_at: 2026-06-02T07:02:45+00:00
source_commit: 9c1a6a00ef413932805da5b813465a7a9d517fb9
watched_paths_digest: 4e11fecf7479fcbd0c57d024f73292eaf9dc2a1aada9b35637edc3771e248649
source_doc: docs/language-reference/basics-program-execution.md
source_doc_digest: 2def7b8eb408ab4fcbbb9d248b212532689d112c97c0dc9ddc39e29a0517e0bd
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for conformance/basics-program-execution

## Intent

Tests verify execution-model claims in the **language reference** at
[`docs/language-reference/basics-program-execution.md`](../../../../language-reference/basics-program-execution.md).
The source doc (~102 lines, three sections) is primarily a conceptual
execution-model description. Claims are extracted by identifying
sentences that name a Slang keyword, intrinsic, or attribute whose
presence is observable in emitted code (HLSL + SPIR-V emission pairs)
or in CPU compute dispatch results. Pure model statements (wave shapes,
lockstep semantics, scheduling) are classified as non-normative and
recorded in Untested claims.

## Claims

### Section: Program Execution

- **C1** Workload for a Slang program is dispatched (compute) or launched (graphics). _(non-normative model intro)_
- **C2** The dispatched/launched workload is divided into entry point invocations executed by threads. _(non-normative)_
- **C3** A compute dispatch is a 3D set of grid points `g` with `0 <= g.x < grid_dim.x`, `0 <= g.y < grid_dim.y`, `0 <= g.z < grid_dim.z`.
- **C4** For every grid point, a thread group is instantiated; within it, thread positions `b` satisfy `0 <= b.x < group_dim.x`, `0 <= b.y < group_dim.y`, `0 <= b.z < group_dim.z`. Thread group dimensions are typically specified via an attribute on the compute entry point.
- **C5** Total invocations per dispatch = `grid_dim.x * grid_dim.y * grid_dim.z * group_dim.x * group_dim.y * group_dim.z`.
- **C6** Individual invocations are grouped into waves; wave size is a power-of-two in [4, 128] and is defined by the target. `WaveGetLaneCount()` queries this size.
- **C7** In compute dispatches, a wave is subdivided from a thread group in a target-defined manner; the application should not assume wave shapes. _(non-normative advisory)_
- **C8** Some waves may be only partially filled when thread group or launch does not align with wave size. _(non-normative advisory)_
- **C9** Thread group size should generally be a multiple of wave size for best utilization. _(non-normative performance advisory)_

### Section: Thread Group Execution Model

- **C10** All threads within a thread group share local memory allocated with the `groupshared` attribute.
- **C11** `GroupMemoryBarrier()` and `GroupMemoryBarrierWithGroupSync()` are related barriers for thread-group shared memory.
- **C12** The thread group execution model applies only to compute kernels. _(execution-model statement; Slang does not gate `groupshared` syntax at compile time)_

### Section: Wave Execution Model

- **C13** All threads in a wave execute in the SIMT model. _(non-normative remark)_
- **C14** Wave-tangled functions include ballots, reductions, shuffling, and control-flow barriers with the wave scope.
- **C15** Active thread: participates in producing a result. _(model definition; not directly testable via slang-test)_
- **C16** Inactive thread: does not produce side effects; can be inactive because not on the current path, wave underutilized, or `discard` executed (fragment only).
- **C17** Helper thread: computes derivatives (fragment quads); does not produce other side effects; does not participate in wave-tangled functions unless stated. _(GPU-derivative-only; non-observable without ddx/ddy runtime context)_
- **C18** Slang does not require wave invocations to execute in lockstep unless on a mutually convergent control-flow path and executing synchronizing functions. _(non-normative model statement)_

## Functional coverage

| Claim                                                                                                                                                                                | Intent     | Anchor                                                                                                                   | Tests                                                                                      |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------- | ------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------ |
| C3: Thread coordinates `SV_DispatchThreadID`, `SV_GroupID`, `SV_GroupThreadID` are emitted for a compute entry point receiving the 3D grid position.                                 | functional | [#program-execution](../../../../language-reference/basics-program-execution.md#program-execution)                       | [`dispatch-thread-id-emission.slang`](dispatch-thread-id-emission.slang)                   |
| C4: `[numthreads(x,y,z)]` attribute on a compute entry point is preserved in emitted HLSL and SPIR-V (LocalSize).                                                                    | functional | [#program-execution](../../../../language-reference/basics-program-execution.md#program-execution)                       | [`numthreads-attribute-emission.slang`](numthreads-attribute-emission.slang)               |
| C4, C5: With `numthreads(4,1,1)` and a 1-group dispatch, threads receive `SV_DispatchThreadID` values 0..3 (within [0, group_dim.x)).                                                | functional | [#program-execution](../../../../language-reference/basics-program-execution.md#program-execution)                       | [`dispatch-invocation-count-functional.slang`](dispatch-invocation-count-functional.slang) |
| C3, C4: `SV_GroupID` is within [0, grid_dim) and `SV_GroupThreadID` is within [0, group_dim) for each axis.                                                                          | functional | [#program-execution](../../../../language-reference/basics-program-execution.md#program-execution)                       | [`dispatch-3d-group-coords-functional.slang`](dispatch-3d-group-coords-functional.slang)   |
| C6: `WaveGetLaneCount()` compiles in a compute entry point; emits `WaveGetLaneCount` in HLSL and `SubgroupSize` built-in in SPIR-V.                                                  | functional | [#program-execution](../../../../language-reference/basics-program-execution.md#program-execution)                       | [`wave-lane-count-emission.slang`](wave-lane-count-emission.slang)                         |
| C10: `groupshared` attribute is emitted as `groupshared` in HLSL and as Workgroup storage class in SPIR-V for a compute entry point.                                                 | functional | [#thread-group-execution-model](../../../../language-reference/basics-program-execution.md#thread-group-execution-model) | [`groupshared-memory-functional.slang`](groupshared-memory-functional.slang)               |
| C11: `GroupMemoryBarrier()` and `GroupMemoryBarrierWithGroupSync()` compile in a compute entry point; emit named HLSL barriers and `OpControlBarrier` / `OpMemoryBarrier` in SPIR-V. | functional | [#thread-group-execution-model](../../../../language-reference/basics-program-execution.md#thread-group-execution-model) | [`group-memory-barrier-emission.slang`](group-memory-barrier-emission.slang)               |
| C14: `WaveActiveBallot` (ballot wave-tangled function) compiles in a compute entry point; emits `WaveActiveBallot` in HLSL and `OpGroupNonUniformBallot` in SPIR-V.                  | functional | [#wave-execution-model](../../../../language-reference/basics-program-execution.md#wave-execution-model)                 | [`wave-tangled-ballot-emission.slang`](wave-tangled-ballot-emission.slang)                 |
| C14: `WaveActiveSum` (reduction wave-tangled function) compiles in a compute entry point; emits `WaveActiveSum` in HLSL and `OpGroupNonUniformIAdd` in SPIR-V.                       | functional | [#wave-execution-model](../../../../language-reference/basics-program-execution.md#wave-execution-model)                 | [`wave-tangled-reduction-emission.slang`](wave-tangled-reduction-emission.slang)           |
| C16: The `discard` statement disables a fragment thread (making it inactive); emits `discard` in HLSL and `OpKill` in SPIR-V for a fragment entry point.                             | functional | [#wave-execution-model](../../../../language-reference/basics-program-execution.md#wave-execution-model)                 | [`discard-inactive-thread-emission.slang`](discard-inactive-thread-emission.slang)         |

## Untested claims

| Claim                                                                                                                                       | Reason          | Anchor                                                                                                                   | Why untested                                                                                                                                              |
| ------------------------------------------------------------------------------------------------------------------------------------------- | --------------- | ------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C1: Workload is dispatched (compute) or launched (graphics).                                                                                | (unclassified)  | [#program-execution](../../../../language-reference/basics-program-execution.md#program-execution)                       | Pure model introduction; no Slang keyword or intrinsic to observe.                                                                                        |
| C2: The workload is divided into entry point invocations executed by threads.                                                               | (unclassified)  | [#program-execution](../../../../language-reference/basics-program-execution.md#program-execution)                       | Conceptual description; the compute system-value tests (C3, C4, C5) cover the observable surface.                                                         |
| C7: In compute dispatches, a wave is subdivided from a thread group in a target-defined manner; applications should not assume wave shapes. | (unclassified)  | [#program-execution](../../../../language-reference/basics-program-execution.md#program-execution)                       | Advisory sentence about scheduling; no observable compiler surface.                                                                                       |
| C8: Some waves may be partially filled when the thread group does not align with wave size.                                                 | (unclassified)  | [#program-execution](../../../../language-reference/basics-program-execution.md#program-execution)                       | Target-scheduling detail; not observable via slang-test.                                                                                                  |
| C9: Thread group size should generally be a multiple of wave size for best utilization.                                                     | (unclassified)  | [#program-execution](../../../../language-reference/basics-program-execution.md#program-execution)                       | Performance advisory; no testable compiler behavior.                                                                                                      |
| C12: The thread group execution model applies only to compute kernels.                                                                      | (unclassified)  | [#thread-group-execution-model](../../../../language-reference/basics-program-execution.md#thread-group-execution-model) | Slang does not gate `groupshared` syntax at compile time for other stages; this is an execution-model semantic statement, not a compile-time restriction. |
| C13: All threads in a wave execute in the SIMT model.                                                                                       | (unclassified)  | [#wave-execution-model](../../../../language-reference/basics-program-execution.md#wave-execution-model)                 | Remark 1 notes SIMT may not apply to CPU targets; this is a model description, not an observable claim.                                                   |
| C15: Active thread participates in producing a result.                                                                                      | (unclassified)  | [#wave-execution-model](../../../../language-reference/basics-program-execution.md#wave-execution-model)                 | Model definition; no intrinsic exposes active-thread status directly.                                                                                     |
| C17: Helper thread computes derivatives (fragment quads); does not participate in wave-tangled functions unless stated.                     | gpu-non-compute | [#wave-execution-model](../../../../language-reference/basics-program-execution.md#wave-execution-model)                 | Observing helper-thread vs active-thread distinction requires GPU fragment derivative tests; no CPU path exercises this.                                  |
| C18: Wave invocations do not require lockstep unless on a mutually convergent path and executing synchronizing functions.                   | (unclassified)  | [#wave-execution-model](../../../../language-reference/basics-program-execution.md#wave-execution-model)                 | Non-normative model statement about what Slang does not require; not testable via compiler output or compute buffer values.                               |

## Doc gaps observed

NA
