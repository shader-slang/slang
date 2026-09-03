# Slice 186: Plan canonical memory and resource operations once

## Motivation

Consider `InterlockedAdd` on an `RWStructuredBuffer<int>` beside an `RWTexture2D<float>` load.
Final NVVM-ready IR contains a canonical atomic instruction and an explicit `ImageLoad`. The first
preflight walk already proved their writable pointer/resource producer, types, address space,
memory order, coordinates, format, and exact provider descriptor. Later validation and emission
nevertheless reran the same resolvers, and pointer validation secretly did so once more just to
recognize an atomic consumer.

## Proposed solution

Add family-specific emission-plan records for ordinary atomics, explicit image surfaces, default
resource values, and canonical ephemeral values. Store the exact resolver result once and thread
the immutable plan into pointer validation. Build all emitter source indexes through one checked
typed helper.

## Change summary

- Added typed planned atomic, surface, default-resource, and ephemeral records.
- Made first-pass preflight append each accepted source record while retaining deduplicated
  capability queries.
- Made SSA, pointer-consumer validation, and emission consume records directly.
- Consolidated nine identical source-index construction loops into one typed helper.

## Concepts and vocabulary

**Ordinary atomic** is a canonical atomic IR instruction, distinct from GenericAsm byte-address
and reduction helper recipes. **Ephemeral value** is a chosen undefined value, stable literal hash,
or ignorable debug-scope marker whose meaning is already explicit in final IR. **Plan index** maps
one canonical source instruction to its owned family record.

## Process report

`_resolveNVVMAtomicOperation` proves an exact writable pointer producer, relaxed order literals,
physical scalar type, address space, operands, and optional subtract/implicit-value recipe.
`_resolveNVVMSurfaceImageOperation` proves the PTX-image-legalizer output and call-site storage
format. `_resolveNVVMDefaultResourceValue` handles only optional-none structured-buffer or
descriptor leaves. `_resolveNVVMEphemeralValue` handles three exact canonical opcodes. Those
producer shapes were already valid and intentionally supported, so this slice retains their
resolvers at the first preflight boundary rather than changing upstream IR.

The resolver results now own a source key and all fields later consumers need. Default structured
buffers retain only their semantic element type rather than copying the broader temporary
`NVVMRawBufferType`; that is the precise data emission consumes. Explicit surface plan records own
their descriptor and diagnostic, eliminating the separate source lookup during ordinary image
emission. GenericAsm surface helpers keep their existing requirement lookup and are not silently
folded into this source-instruction contract.

`_validatePointerValue` previously invoked `_resolveNVVMAtomicOperation(consumer, ...)` to permit a
writable structured-buffer element pointer. The caller now supplies the immutable requirements,
and the validator checks the planned atomic pointer identity. This is not a new acceptance case:
first-pass preflight must already have proven the complete atomic before this walk begins.

The self-review inventory contains four plan structures, four first-pass append sites, the plan
parameter on pointer validation, four validation consumers, four emitter indexes/consumers, and
the shared index helper. All survive because they remove a repeated semantic decision or duplicated
checked indexing. No fallback, syntax reconstruction, adjacent shape, or provider callback was
added. Each migrated resolver now occurs only at its definition and first-pass use.

The Release build and focused ephemeral/atomic/surface tests pass. The selected prefix passes
437/437 and the permanent category passes 92/92. Frozen corpus v1 remains 452 workloads/427 healthy
references at 418/418/418 O0/O3/both with zero old-correct regression; all-row direct results stay
432 correct, three runtime mismatches, and 17 preflight failures per mode. Discovery remains 82/72
at 72/72/72; classifications stay 72 correct, seven infrastructure, one runtime mismatch, and two
preflight failures per mode. Provider ABI revision 34 is unchanged.
