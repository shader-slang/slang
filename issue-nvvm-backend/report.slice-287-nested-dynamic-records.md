# Slice287: nested records through dynamic dispatch

## Motivation

Flat FP8/BF16 dynamic dispatch passed in270, and nested local records passed in279. Neither result
proved their combination. This slice exercises nested fields through runtime-selected interface methods,
including a mutating method and an earlier interface-value copy:

```slang
[anyValueSize(20)] interface IRecord
{
    uint inspect(uint bits, uint expectedKind);
    [mutating] void flip();
};
IRecord object = createDynamicObject<IRecord>(inputBuffer[slot], packed);
uint before = object.inspect(bits, kind);
let snapshot = object;
object.flip();
uint after = object.inspect(bits ^ 0x5aa5, kind);
uint saved = snapshot.inspect(bits, kind);
```

`packed` contains five independently constructed uint words. Two conformers reorder a nested
FP8/FP8/BF16 leaf and a guarded BF16 pair between integer head/tail fields. A second fixture substitutes
integer leaves with the same widths and layout. Every inspection checks all nine fields and the
conformer identity against integer expressions derived from the input, without floating arithmetic.

## Proposed solution

Qualify the existing implementation on accepted285 binaries. All six new GPU cells pass at NVRTC O3,
NVVM O0 and NVVM O3. Each conformer traverses all65,536 encodings; every FP8 encoding and both BF16
lanes are covered, with XOR-correlated inputs rather than a Cartesian product. Each full output is
`[0,0,0,65536,0,0,0,65536]`: initial, mutated and saved-value mismatch masks plus completion count.

Two deliberate payload corruptions are rejected, producing the independently predicted masks4 and2
in all three inspections for the affected conformer. All six unchanged flat-dynamic/nested-local
neighbor cells pass. Two emitted-IR captures confirm runtime selection and live packing/unpacking.
No compiler fix is needed for this bounded domain.

## Change summary

Only this report, the completed plan, compact [evidence](research-evidence.slice-287.json) and navigation
are committed. Frozen sources, independent layout/oracle, serial runner, complete outputs, captures
and reviews remain under `build/nvvm-nested-dynamic287`. No production or maintained-test change.
Full285 validation is inherited:580cases/576sources/1740cells,1703correct/37unresolved/20histories.
All100 installed layout entries,37runtime/11qualifiedsource/2config/576input hashes and22pins remain exact.

## Concepts and vocabulary

**AnyValue payload** is the20-byte Natural packing used by the interface representation.
**Concrete local record** uses CUDA layout and occupies24bytes. For example, the guarded pair has
before/pair/after offsets0/2/6 in Natural layout and0/4/8 in CUDA layout. A payload offset is therefore
not a physical local offset. **Pack-back** converts a concrete mutated receiver back into the interface
payload before its next inspection; the earlier interface copy must retain its original payload.

## Process report

A fresh bounded author prepared two conformers, independent integer expectations and two corruption
controls before execution. A separate reused reviewer derived the layouts and traced the canonical
API. `createDynamicObject` produces `CreateExistentialObject`; `lowerCreateExistentialObject` creates
the runtime-witness tuple. `maybeUnpackArg` handles the mutating receiver's `BorrowInOut` copy-in/out,
and `emitMarshallingCode` recursively visits canonical fields under Natural layout rules. These are
valid canonical aggregate inputs; no alternative representation, fallback or custom marshaller was added.

Both captured LLVM modules load the kind as `%24`, resolve its dispatch ID `%66`, and use four live
switches. Initial inspection unpacks original payload `%67`. Each mutation branch unpacks the concrete
record, calls its `flip`, and repacks it; merged payload `%96` feeds the later inspection. The snapshot
inspection still consumes `%67`. The concrete aggregate types retain CUDA padding while pack/unpack
helpers address the five-word Natural payload. This proves the intended live path rather than merely
finding unused generated helpers.

All16 frozen obligations completed serially without timeout or retry. Native counts and every output
word were checked independently of the maintained classifier. The two intentional failures retain its
raw `unclassified` label; their FileCheck rejection and complete buffers establish expected oracle
rejection, not passing shader results. No main-corpus obligation or old failure was removed.

Coverage excludes record arrays, whole BF3/BF4 values, device/shared/readonly/exported record roles,
arithmetic accuracy and performance. The next bounded slice can promote these successful raw probes
into persistent native regressions and update the qualified record contract, retaining the separate
Natural/CUDA layouts and all current exclusions. The authorized development loop continues; skip Slack.
