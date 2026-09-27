# Evaluate corrections for nested-array padding loss

## Motivation

Research280 exposed incorrect copies of `Cell { uint16 first; Child { uint16 first; uint last; } }`
arrays in both optimized backends. The three leaves belong at offsets0,4,8 with stride12; ordinary
whole copies incorrectly place the child.first value at offset2. A correction must preserve snapshots
without emitting one compiler IR operation per element of a potentially large array.

## Proposed solution

No production change is selected. Standalone LLVM/CUDA prototypes compare three correction families
with failing ordinary-copy controls. Each small GPU cell uses three elements and65536low16-bit patterns,
checks every source/destination/returned-old field, then repeats with exact source==destination.
The independent full-buffer oracle is `[0,123,0,456]`.

| Copy operation                | NVRTC O3   | NVVM O0 | NVVM O3   |
| ----------------------------- | ---------- | ------- | --------- |
| Ordinary whole copies         | Wrong9234  | Pass    | Wrong9234 |
| Pointer memcpy with snapshots | Pass       | Pass    | Pass      |
| Pointer canonical field loop  | Pass       | Pass    | Pass      |
| Typed SSA helper, optnone     | Not tested | Pass    | Pass      |

Only the small helper carries `noinline optnone`; its caller stays compiled atO3. That attribute is
listed by the [NVVM12.9 specification](https://docs.nvidia.com/cuda/archive/12.9.1/nvvm-ir-spec/index.html#function-attributes)
and is effective in this installed compiler. It is not a whole-module O0 comparison.

Both pointer families compile for65536elements in all three modes. LLVM instruction counts remain
102 for memcpy and114 for field loops; optimized PTX grows by only one instruction in each backend.
Storage grows: local depot declarations change from36/108bytes to786432/2359296bytes. These large
modules were never launched, so compact code does not establish runtime feasibility or performance.

## Change summary

This report, completed plan, [structured evidence](research-evidence.slice-281.json) and navigation
updates retain23attempt cells:17effective cells (nine correct GPU, two failing baseline GPU, six large
compile-only) plus six preserved declaration failures. Raw sources, runners, PTX and output records
remain under `build/nvvm-array-store-prototype281`. No compiler/provider/harness/corpus change or rebuild.
All37runtime artifacts,8qualified sources,2configs,576main inputs and22pins remain exact279.
Full279/targeted233/cadence0 and its1740outcomes/37unresolved/20resolved histories are inherited;
research280's optimized nested-array failures remain open.

## Concepts and vocabulary

An SSA aggregate is a value captured at a particular program point; rereading its old source address
later may return different data. Pointer-copy loops require a correctly materialized snapshot address.
A typed helper accepts that SSA value directly. `optnone` isolates an optimization boundary; it does
not repair or replace the canonical type representation.

## Process report

Both ordinary-copy controls reproduce padding loss with wrong mask9234, identifying child.first in
destination, old snapshot and both self-copy checks. The source types are valid, all relevant fields
are initialized and indices are bounded. The vendor defect remains downstream of valid emitted code.

Pointer candidates copy old←destination, temporary←source, then destination←temporary. Each individual
memcpy pair is disjoint, including exact self-assignment; the snapshot finishes before mutation. The
leaf loop uses typed field addresses at0/4/8, a runtime count, stride12 and a retained PTX backedge.
The initial memcpy prototypes failed before execution: the legacy NVVM parser rejects `immarg`, and
NVRTC already supplies a memcpy declaration. A separately frozen second attempt removes only those
declarations; all six corrected cells succeed. Original failures are retained rather than counted as
GPU passes. Every process completed within its120-second bound; elapsed times are operational records,
not performance comparisons.

The SSA candidate loads both previous/incoming aggregates before either store, passes each to a typed
helper and leaves the caller optimized. Actual PTX retains the calls and stores helper fields at
0/4/8 in each element; no faulty0/2 store appears. This establishes only the small aligned example.
Large helpers, underaligned roots and other SSA producers remain unqualified.

Existing `lowerCopyLogicalWithDestImpl` recurses through canonical fields and emits loops above16
array elements, but currently serves SPIRV pointer-to-pointer lowering. It cannot reconstruct an earlier
SSA snapshot by rereading mutable memory. The provider's dynamic array extraction uses one constant
extract/select per element; placing that inside a source-level loop would still expand IR. Spilling
an SSA value with the affected whole store is circular. Any production design must resolve this
producer/consumer boundary, not merely transplant the passing pointer loop.

Independent reused-context review and root audits check sources, snapshot/self-alias contracts,
complete outputs, caller/helper PTX and identities. Limits are runtime seed0/count3, correlated16-bit
patterns, aligned local storage and exact self-alias only. Large pointer compilations establish code
size, not stack capacity; no material GPU or speed claim. Next: qualify the typed SSA-helper boundary
for other producers, underalignment and larger compile sizes before selecting a bounded NVVM fix.
NVRTC's open defect remains a separate production obligation. The loop continues; skip Slack.
