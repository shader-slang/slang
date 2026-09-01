# Slice 183: Cache one NVVM type-role classification

## Motivation

Every call to direct-NVVM type lowering rebuilt more than fifty overlapping scalar, aggregate,
resource, pointer, and storage predicates before checking its requested role. Recursive aggregate
lowering repeated the work, while the role-admission matrix lived separately from the values it
classified.

## Proposed solution

Classify each canonical linked-IR type once per module emission context. Store all resolved shapes
and role facts in `NVVMTypeInfo`, put the complete admission matrix in `NVVMTypeInfo::supports`, and
make provider type construction consume that record without changing any physical ABI.

## Change summary

- Added the `NVVMTypeInfo` classification record and role query.
- Added a module-emission-context cache keyed by canonical `IRType*`.
- Moved the nine-role admission matrix out of `lowerType` and into the record.
- Reused resolved widths, aggregate forms, resource descriptors, and pointer pointees during type
  construction.
- Rejected a universal aggregate-memory ABI after auditing established representation roles.

## Concepts and vocabulary

**Type role** is the producer/consumer contract under which the same Slang type is represented:
entry-point ABI, helper ABI/value, ordinary SSA value, ordinary storage, parameter-group storage,
or structured-buffer storage. **Classification cache** stores canonical Slang type facts, not
provider handles; provider handles remain separately cached for each physical representation.

## Process report

Consider a struct containing a device pointer passed through a helper. The canonical Slang struct
is a finite helper value whose pointer leaf must lower to LLVM's generic address space: a caller may
provide either a kernel global pointer or `__getAddress` of local storage. The same struct used as
launch/storage data has a different provenance and may require global pointers or CUDA aggregate
layout. A provider handle created for one of those roles therefore cannot prove another role legal.

Previously `NVVMTypeLoweringContext::lowerType` queried every classifier into local variables,
built one large `isLegal` expression, and only then selected a role-specific handle cache. Recursive
array, struct, pointer, and parameter-group lowering entered the same classifier block again.
`_getTypeInfo` now resolves those exact canonical classifiers once, including pointer pointees,
resource descriptors, scalar widths, aggregate forms, and storage properties. It caches the
record by the canonical `IRType*`. `NVVMTypeInfo::supports` owns the full role matrix, so a cached
provider handle still cannot bypass admission.

The aggregate-as-memory prototype gate was deliberately not promoted. Copyable aggregates already
cross helper boundaries as first-class values; resource aggregates contain opaque handles;
parameter-group and structured-buffer aggregates follow distinct CUDA layouts; LLVM 14 typed
pointers need the role-specific pointee. Forcing all aggregates through memory would erase those
canonical distinctions and add transport without addressing one of the five remaining frozen
helper-ABI failures. The correct next step is to use the shared classification when a concrete
producer shape is generalized, not pre-emptively replace every aggregate ABI.

The self-review inventory contains one record, one cached construction function, one role switch,
and the existing `lowerType` consumers. No classifier was widened, no malformed representation was
admitted, no source syntax was reconstructed, and no fallback or provider callback was added.
Provider ABI revision 34 is unchanged.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references at 418/418/418 O0/O3/both,
with no old-correct regression. All-row direct classifications remain 432 correct, three runtime
mismatches, and 17 preflight failures per mode. Discovery remains exactly 82 workloads/72 healthy
references at 72/72/72. The selected prefix passes 437/437 and the permanent NVVM category passes
92/92.
