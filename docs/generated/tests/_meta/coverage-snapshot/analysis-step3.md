# Step 3 analysis — deep emission-variant expansion + doc-enrichment backlog

**Goal of this step:** push doc-grounded depth to its limit on the highest-
residual emission bundles, and produce the concrete **doc-enrichment backlog**
for the coverage that is genuinely beyond what the current docs describe.

## What shipped

Exhaustive depth-first expansion (`.claude/wf-expand-deep.js`) over
`target-pipelines/{spirv,hlsl,metal,cuda,wgsl}`, instantiating every
doc-enumerable emission variant (per element type / shape / op) and emitting to
spirv-asm **and** glsl on the shared SPIR-V/GLSL legalize path.

- **+79 tests** across 5 per-bundle commits (`7e395aa58`…`c46b6c335`), all
  `intent=expansion`, CHECK lines pinned to real slangc output. verify FAILED:0,
  lint 0/0. (spirv +16: full atomics op-family, bitcast, ArrayStride per
  type/shape, first GLSL-arm tests; hlsl +17; metal +10; cuda +16; wgsl +20.)

## Coverage result — full trajectory

| stage                        | line cov   | covered lines | Δ        |
| ---------------------------- | ---------- | ------------- | -------- |
| baseline (pre-regen)         | 53.10%     | 134,016       | —        |
| after regen (low point)      | 51.77%     | 130,665       | −3,351   |
| + backend fan-out            | 52.00%     | 131,242       | +577     |
| + Step 2 (variant breadth)   | 52.06%     | 131,410       | +168     |
| **+ Step 3 (deep emission)** | **52.27%** | **131,922**   | **+512** |

**Step 3 alone: +512 lines** (3× Step 2 — depth-first beats breadth-first here).
Floor raised to **131,922**. Total recovered from the low point: **+1,257 lines**
(~37% of the 3,351 lost). Remaining gap vs baseline: **−2,094 lines (−0.83pp)**.

## Why we stop at −0.83pp — and the doc-enrichment backlog

The remaining ~2,094 lines are **doc-limited**: the design docs name a _general_
rule but do not _enumerate_ the variant families the compiler branches on.
Step 3's agents recorded these as `## Doc gaps observed` — this is the
actionable backlog. To close the rest, the **design docs must be enriched**
(through the design-doc regen channel — I did NOT hand-edit the generated SOT
unattended, as that is circular and breaks its freshness/review provenance).

**9 doc gaps to enrich (then expand tests against):**

1. **SPIR-V atomics op-family** — doc names only `InterlockedAdd→OpAtomicIAdd`;
   code handles And/Or/Xor/Exchange/CompareExchange and the signed/unsigned
   Min/Max split (`OpAtomicSMin/UMin`…). _(tests already added against the
   observed behavior; doc should document the family.)_
2. **SPIR-V StructuredBuffer std430 aggregate layout** — only `*_std140` for
   cbuffer is documented; `StructuredBuffer<struct>` emits a `*_std430`
   StorageBuffer layout with per-member `Offset`.
3. **HLSL byte-address Load is element-type-dependent** — bare `.Load` (native
   uint word) vs templated `Load<T>` vs per-component vector decompose.
4. **HLSL composite-select** — doc names matrix/struct; array-typed conditional
   is rewritten identically but unlisted.
5. **Metal matrix device/structured buffers** — `_MatrixStorage_` wrapping is
   documented only for cbuffer, but `RWStructuredBuffer<float4x4>` is wrapped
   too (via `lowerBufferElementTypeToStorageType`).
6. **Metal atomic operand family** — wider than documented `atomic_uint`:
   `atomic_int/ulong/long/float`, device vs threadgroup address space;
   half/double rejected at type-check (E36107).
7. **CUDA Interlocked→atomic\* mapping** — full family
   (`atomicAnd/Or/Xor/Min/Max/Exch/CAS`) + operand-type × storage-target
   orthogonality, beyond the documented `atomicAdd`.
8. **WGSL numeric width matrix** — i64/u64 and f16 (with emitted `enable f16;`)
   are supported & observable; **`double` aborts with an internal error**
   instead of a clean diagnostic.
9. **WGSL narrow ints** — `int16/uint16` give clean diag E56103, but
   **`int8/uint8` abort with an internal error** instead.

## Bonus: 2 compiler bugs surfaced by the depth work

Items 8 & 9 are not just doc gaps — they are **compiler bugs**: WGSL emit of
`double` ("unexpected: double type emitted") and `int8/uint8` ("unexpected: 8
bit integer type emitted") hit internal-error aborts rather than emitting a
clean unsupported-type diagnostic like the 16-bit path does. Recommend filing
these as findings (`_meta/findings/`) — they were not turned into tests (the
aborts would fail), so they sit in the bundle Untested/Doc-gaps for now.

## Step 3 outcome

+512 lines to 52.27% (floor 131,922); doc-grounded depth essentially exhausted;
the −0.83pp remainder is captured as a precise 9-item doc-enrichment backlog
plus 2 compiler-bug findings — the actionable path to full parity, to run with
a human in the loop via the design-doc regen.
