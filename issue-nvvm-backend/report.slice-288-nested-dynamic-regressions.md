# Slice288: preserve nested dynamic record regressions

## Motivation

Slice287 proved nested FP8/BF16 dynamic unpacking, mutation pack-back and interface snapshot
preservation, but its probes lived only under ignored build artifacts. A compiler regression could
therefore escape the normal native test suite. This slice makes both the substandard fixture and its
integer structural control persistent tests.

## Proposed solution

Add three explicit directives to each promoted fixture: NVRTC O3, NVVM O0 and NVVM O3. Preserve the
proven executable bodies and input/expected-output directives. Each conformer traverses65,536
correlated encodings, independently checks nine fields and its identity, mutates the object and checks
both the changed value and an earlier snapshot. Eight nonzero-initialized outputs must become
`[0,0,0,65536,0,0,0,65536]`.

All six fresh native cells pass on unchanged accepted285 binaries. The two deliberate corruption
controls, two live-path IR captures and six neighboring passes remain inherited287 evidence, with
original identities. No compiler change, rebuild or repeated full checkpoint was needed.

## Change summary

- `tests/cuda/nvvm-nested-substandard-dynamic.slang` preserves FP8/BF16 nested dynamic coverage.
- `tests/cuda/nvvm-nested-integer-dynamic.slang` preserves the same shape with integer leaves.
- `docs/design/nvvm-substandard-record-contract.md` documents the qualified dynamic path and separate
  Natural/CUDA layouts. All earlier exclusions and array-store limitations remain.
- Compact [evidence](research-evidence.slice-288.json), completed plan and navigation retain validation
  and provenance. Raw artifacts stay under `build/nvvm-nested-dynamic288`.

## Concepts and vocabulary

**Natural payload** is the20-byte five-word interface packing; **CUDA local layout** allocates24bytes
for either concrete record. **Pack-back** converts the mutated concrete receiver into the interface
payload. **Snapshot** is the earlier interface copy whose original payload must remain unchanged.
These terms retain the contracts established in287; no new representation is introduced.

## Process report

The motivating operation is:

```slang
IRecord object = createDynamicObject<IRecord>(kind, packed);
let snapshot = object;
object.flip();
uint changed = object.inspect(bits ^ 0x5aa5, kind);
uint preserved = snapshot.inspect(bits, kind);
```

`lowerCreateExistentialObject` creates the runtime-witness/payload tuple. Generated wrappers use
`emitMarshallingCode` to unpack canonical fields; `maybeUnpackArg` handles a mutating `BorrowInOut`
receiver and its repacking. The input shape is intentional and canonical, with distinct Natural and
CUDA layouts. No producer fix, helper, fallback or special case is added. Existing287 IR proves live
runtime selection, mutation repacking and use of the original payload for the snapshot.

A fresh bounded author promoted the sources; an independent reused reviewer checked equivalence and
scope. Root and reviewer independently matched executable tokens and every TEST_INPUT/CHECK directive
after formatting. The only harness addition is explicit mode coverage. Native ordinal selection
runs `.0`, `.1`, `.2` separately with180-second limits and retries disabled. Each cell reports exactly
one executed/passed test, zero ignored and the complete expected buffer. No failure or timeout occurred.

Before/after identities preserve100installed entries,37runtime/11qualifiedsource/2config/576maininput
hashes and22pins. Accepted285 full outcomes remain inherited:580cases/576sources/1740cells,
1703correct/37unresolved/20histories; discovery stays128. These two native sources add six focused
cells outside that main corpus. A post-execution documentation clarification labels field-only
excerpts; the exact execution-time contract text is retained separately. No shader bytes changed.

The tests cover raw encodings with correlated inputs, not Cartesian combinations or floating arithmetic.
No arrays, whole BF3/BF4 values, new resource/pointer roles, arbitrary alignment/address spaces or cache
visitation-order guarantee is added. The authorized loop continues with evidence-led language breadth;
material overload-screening remains a frequency lead, not an accepted optimization. Skip Slack.
