# Slice 184: Establish a plan-producing NVVM preflight

## Motivation

Direct preflight already resolved ordinary provider operations and discovered the reachable helper
closure, but operand validation and emission repeated those decisions. A common arithmetic or cast
instruction was interpreted up to three times, and physical function names were regenerated after
all capability checks.

## Proposed solution

Extend the existing provider-requirement result with an owned `NVVMEmissionPlan`. Record stable
function order, physical names, and one source-keyed typed descriptor for each ordinary value
operation during the first preflight pass. Make later validation and emission consume the plan.

## Change summary

- Added `NVVMEmissionPlan` and `NVVMPlannedValueOperation`.
- Retained deduplicated overload requirements for provider capability queries.
- Recorded source-keyed descriptors for arithmetic, comparisons, conversions, selects, fixed wave
  values, and ordinary bit reinterpretation.
- Removed repeated value-operation resolution from operand validation and emission.
- Removed repeated reachable-function and physical-name collection from emission.

## Concepts and vocabulary

**Capability requirement** is one deduplicated typed provider overload queried before module
creation. **Emission record** is one canonical source instruction paired with its already selected
descriptor. **Plan-producing preflight** means validation returns these stable decisions as data;
it does not mean preflight mutates or invokes the provider.

## Process report

Consider an ordinary selected integer add in a helper. `_validateNVVMFunction` first proves its
complete operand/result signature through `_resolveNVVMValueOperation`. Before this slice it copied
the descriptor only into a deduplicated capability list. The operand-availability walk invoked the
resolver again, and `emitNVVMIRFromLinkedIR` invoked it a third time immediately before calling
`emitValueOperation`.

`_planNVVMValueOperation` now copies the selected operation, result type, operand types, diagnostic,
and source `IRInst*` into owned plan storage. It separately adds the overload to the existing
deduplicated capability list. This duplication is semantic rather than accidental: the provider
should be queried once per overload, while emission needs one record per source instruction. The
copy is necessary because `SlangNVVMValueOperationDesc::operandTypes` may point into resolver-local
storage; the plan never retains that pointer.

The operand-validity pass now asserts that the canonical instruction has a planned descriptor and
checks only SSA operand availability. Emission builds a source-to-plan index once and passes the
owned descriptor directly to the provider. There is no `_resolveNVVMValueOperation` call in the
emission half of the file.

The same plan owns the direct-call preorder and collision-checked physical function names produced
by `_collectNVVMFunctions` and `_collectNVVMFunctionNames`. Emission asserts that the first planned
function is the selected entry point and consumes those lists. It no longer repeats either walk.

The migration is intentionally bounded. Numeric truthiness, floating remainder, bitfield recipes,
atomics, resource operations, pointer/addressing relations, aggregates, and GenericAsm compound
recipes carry data beyond the ordinary descriptor and retain their typed paths. They are the
remaining plan-variant inventory; forcing them into this record would recreate a universal target
escape shape.

The self-review inventory contains the plan structs, the descriptor-copy helper, the plan lookup,
four first-pass recording sites, two later validation consumers, the source index, and the function
plan consumers. No operation was widened, no fallback or fixture-name check was added, and provider
capability checks still complete before module creation. Provider ABI revision 34 is unchanged.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references at 418/418/418 O0/O3/both,
with no old-correct regression. All-row direct classifications remain 432 correct, three runtime
mismatches, and 17 preflight failures per mode. Discovery remains exactly 82 workloads/72 healthy
references at 72/72/72. The selected prefix passes 437/437 and the permanent NVVM category passes
92/92.
