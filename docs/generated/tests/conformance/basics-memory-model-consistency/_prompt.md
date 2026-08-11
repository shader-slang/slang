# Prompt: docs/generated/tests/conformance/basics-memory-model-consistency/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/conformance/basics-memory-model-consistency/`,
anchored to
[`docs/language-reference/basics-memory-model-consistency.md`](../../../../language-reference/basics-memory-model-consistency.md).

## Doc summary

The doc (~209 lines, 7 sections) defines the Slang memory consistency model for
multi-threaded (GPU) programs. Claims fall into five normative sections:

1. **Preamble** (lines 1–35): Defines _data race_ and the conditions that avoid
   one: atomicity, same-thread, or happens-before. Cites four sources of
   observable reordering (compiler, hardware, OOO execution, concurrent rate
   variation).

2. **Data Race** (`#data-race`, lines 37–48): One normative definition — two
   accesses conflict when they overlap and at least one is a write. A conflict
   is a data race unless same-thread, atomic, or one happens-before the other.
   The happens-before relation is established by load-acquire observing
   store-release, memory barriers, or Slang Standard Library constructs.

3. **Memory Order** (`#memory-order`, lines 50–109): Five named memory orders
   (Relaxed, Acquire, Release, Acquire-Release, Sequentially-Consistent) with a
   consequence table per order. These are definitional + observable in emission
   (SPIRV memory semantics operands on atomic ops and barriers differ per order).

4. **Atomic Memory Access** (`#atomics`, lines 111–144): Three normative claims:
   (a) all atomic modifications of a single variable occur in a total order
   (serialized); (b) relaxed atomics provide no ordering with other memory
   accesses — only atomicity + variable-local total order; (c) release-acquire
   atomics provide the release/acquire ordering relationships and thread-B
   observes all thread-A side effects before the store. Also: SeqCst implies
   release-acquire + total order for all SeqCst ops. Slang surface: `Atomic<T>`
   and the global `atomic` decls. Advisory: prefer relaxed for multi-target code.

5. **Memory Barriers** (`#barriers`, lines 148–167): Barriers impose
   acquire-release semantics — memory accesses before the barrier happen-before
   those after. Three address-space scopes (All, Device, ThreadGroup). Seven
   named barrier primitives (AllMemoryBarrier, AllMemoryBarrierWithGroupSync,
   AllMemoryBarrierWithWaveSync, DeviceMemoryBarrier,
   DeviceMemoryBarrierWithGroupSync, GroupMemoryBarrier,
   GroupMemoryBarrierWithGroupSync, GroupMemoryBarrierWithWaveSync).

6. **General Programming Guidance** (`#general-programming-guidance`, lines
   169–210): Two advisory patterns — elect first lane for atomics; use
   WaveActiveSum before a single-thread atomic for reductions. The code example
   using `AllMemoryBarrierWithWaveSync` + `WaveIsFirstLane` + `Atomic<T>.store`
   is normatively illustrative (the library functions it invokes are part of the
   claimed surface).

## Claim extraction strategy

- **Definitional "as-if" remarks** (remark 1, 2) are non-normative; skip.
- **Memory order consequence table** rows are normative: each named order is a
  distinct claim about what ordering is enforced. Testable via SPIRV memory
  semantics operands on `OpAtomicStore`/`OpAtomicLoad` — different orders
  produce different `MemorySemantics` operand values.
- **Atomic total-order and release-acquire** claims are runtime-value claims
  only observable on a GPU. Write `COMPARE_COMPUTE -vk/-dx12` tests for CI;
  mark emission tests where observation differs.
- **Barrier primitives** are observable in SPIRV: each maps to
  `OpMemoryBarrier` or `OpControlBarrier` with specific execution scope, memory
  scope, and memory semantics operands. Observable in HLSL as the same-name
  intrinsics. These are the richest emission tests in this bundle.
- **Advisories** (minimize atomics, WaveActiveSum, multi-target relaxed) are
  non-normative recommendations; classify as non-normative in untested claims.
- **AllMemoryBarrierWithWaveSync and GroupMemoryBarrierWithWaveSync** have
  TODO links in the doc; the names appear in the barrier list and in the
  guidance example. Treat as claimed surface; verify emission.

## What NOT to test here

- Actual atomic total-order value behavior (thread A sees 0→1→3 or 0→2→3) —
  requires GPU runner; mark as gpu-other.
- Release-acquire cross-thread observation (thread B observes thread A's side
  effects) — requires GPU runner with coherent inter-thread communication; mark
  as gpu-other.
- Sequentially-consistent total-order across operations — requires multiple GPU
  threads; mark as gpu-other.
- Undefined behavior of data races — no observable signal for UB in slang-test.
