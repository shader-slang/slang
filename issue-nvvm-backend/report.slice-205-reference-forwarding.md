# Slice 205: Forward mutable helper parameters

Status: accepted with a full checkpoint on 2026-09-24.

## Motivation

Both unchanged tiled-brass entries rejected the output of `mx_layer_bsdf` when it called
a mutating `BSDF` method. The relevant source shape is:

```slang
struct BSDF
{
    uint index;
    [mutating]
    void set_layer(int i) { index = i | 0x80000000; }
}
void makeLayer(int layerIndex, out BSDF result)
{
    result = BSDF(0);
    result.set_layer(layerIndex);
}
```

The compiler intentionally gives `result` type `OutParam<BSDF>` and the method's `this` parameter
type `BorrowInOutParam<BSDF>`. Direct NVVM rejected that call even though both pointers reference
the same exact storage. The material's analogous producer is `mx_layer_bsdf`; neither material
source nor its inputs have been modified.

Two executable fixtures independently test this boundary. The first initializes a nested array and
a double field, calls the mutating method twice, and replaces the same object through an `inout`
helper. The output oracle is `[7, 14, 13, 50]`. A scalar helper inside the method forwards its borrow
to an `out int` helper. The second stores an `int*` and an integer in a local helper aggregate,
advances the stored pointer through a mutating method, and replaces the same object. Bound inputs
`[3, 7]` yield `[10, 10]`. This independently exercises the separate pointer-bearing helper branch.
Repeated mutation and replacement observe the original storage rather than a detached temporary.
No claim about aliasing two simultaneously active source borrows is needed.

## Proposed solution

Use the existing canonical local copyable/helper pointer classifiers as the storage contract.
Remove the later argument-op restrictions that demanded `Ptr`, and the redundant helper operand
count check. Keep the existing mutable callee roles and exact `isTypeEqual` pointee comparison.
The classifiers already accept single-operand generic `Ptr<T>`, `OutParam<T>` and
`BorrowInOutParam<T>`; a call can forward between their mutable roles without changing storage.

## Change summary

- `source/slang/slang-emit-nvvm.cpp`: remove redundant argument-spelling restrictions in the two
  existing mutable helper call branches; explain the canonical producer and storage invariant.
- Two `tests/cuda/nvvm-mutable-*-forwarding.slang` fixtures: independent output oracles and exact
  NVRTC O3/NVVM O0/NVVM O3 modes; one disjoint discovery selection for each new source.
- Completed plan, full outcome TSVs, result manifest, design facts and STATUS retain preservation
  evidence, scope and the minimal next-blocker handoff.

## Concepts and vocabulary

A **parameter direction wrapper** is the canonical `OutParam<T>` or `BorrowInOutParam<T>` type
recording a source parameter's passing role. It is distinct from a physical address space or an
alternate element layout. A **copyable value** uses the existing finite numeric/aggregate value
domain. A **helper value** extends that domain to supported pointer/descriptor leaves and finite
aggregates containing them. The **local helper pointer** points to the aggregate itself; pointer
fields inside the aggregate retain their independently established representation. The separate
**direct-resource struct** classifier and explicit thread-local context path are unchanged.

## Process report

`_lowerInfoFromFuncParameters` constructs `OutParam` for source outputs and `BorrowInOutParam` for
mutable parameters, including mutating `this`. `addArg` handles Out/BorrowInOut/BorrowIn by trying
to obtain the existing address. When that succeeds, it passes the address directly with
`addSimpleArg`. For the motivating source, the final IR therefore contains an OutParam-valued
argument to a BorrowInOutParam signature. For replacement it contains BorrowInOutParam to OutParam.
These are intentionally valid canonical parameter roles, not accidental spellings of an element
type. Changing the producer to erase direction information or manufacturing a new `Ptr` would
lose source semantics for no storage benefit.

`_isSupportedNVVMHelperArgument` already asks
`asNVVMSupportedLocalCopyableValuePointerType` or
`asNVVMSupportedLocalHelperValuePointerType` to validate the local storage domain. Both classifiers
accept exactly the three single-operand generic pointer forms above. The copyable path additionally
retains its pre-existing derived-pointer classifier for field/element addresses; that classifier's
layout/address-space domain is unchanged. The parameter must still be OutParam or BorrowInOutParam,
and the argument's pointee must still be exactly equal to the parameter's pointee. Removing the
redundant `Ptr` check admits only the existing canonical mutable wrappers within that proof.

`NVVMTypeLoweringContext` lowers these local copyable/helper parameters with `_lowerPointerType`
and `SLANG_NVVM_ADDRESS_SPACE_GENERIC`. For pointer-bearing helper values it retains the existing
`NVVMTypeUse::HelperValue` representation. The call emitter obtains the already lowered argument
through `_getLoweredNVVMHelperValue` and passes it to `builder.emitCall`; it does not copy, reload,
rebuild, reinterpret or cast it. The separate `_isNVVMGlobalHelperReferenceArgument` path only
casts supported global arguments for the established reference/physical-storage parameter roles.
Our local output/borrow wrappers do not enter that branch. No alias attributes, provider interface,
pointee layout, address-space conversion or frontend lowering changed.

Helper/fallback inventory: (1) remove the copyable branch's redundant Ptr check; (2) remove the
helper branch's redundant Ptr and operand-count checks. Both survive review because separate final
fixtures fail before and pass after their exact boundary. There is no new production helper,
equivalence relation, fallback, arbitrary graph walk, syntax reconstruction or resource classifier.
Each branch's original storage classifier remains the single source of truth. The material is a
copyable aggregate; the independently runnable pointer-bearing fixture justifies the parallel
helper-domain change instead of widening that branch without evidence.

Element identity, layouts, access and address-space exclusions remain enforced at their existing
owners. Focused negative gates rerun `nvvmSlangUnsupportedIRStopsBeforeEmission` (including readonly
Device pointer helper parameters/results and unsupported array pointers),
`nvvmSlangRejectsAdjacentStructuredBufferShapesBeforeProviderMutation` (incompatible aggregate
storage and matrix writes), and `nvvmSlangRejectsReadOnlyByteAddressDataPointerStoreBeforeProviderMutation`.
The existing fake provider's pointee-aware call validation, array-reference tests and stateful
aggregate tests also pass. These are preservation checks, not claims that malformed IR can be
expressed as well-typed source code. Exact element equality and classifier definitions are
byte-unchanged.

Before implementation, each final fixture passed its NVRTC output oracle and rejected the intended
OutParam-to-BorrowInOutParam shape at NVVM O0/O3: two passed and four failed. After the two checks
were removed, the byte-identical sources passed all six cells. This supplies a per-branch removal
proof without another compiler rebuild solely to restore the already recorded failing checks.
The final IR excerpt records both forwarding directions and repeated calls on the same parameter.

Final focused checks pass 14/14, the runtime smoke gate passes 4/4, units pass 473/473 with one
Windows-only skip, and toolkit checks pass 18/18. Shared helper call admission triggers a full
checkpoint under WORKFLOW, despite the bounded implementation. All corpus suites used four workers
and ran sequentially after the small runtime gate.

| Corpus                                            | Identities | NVRTC O3 correct | NVVM O0 correct | NVVM O3 correct |
| ------------------------------------------------- | ---------: | ---------------: | --------------: | --------------: |
| Frozen full selection                             |        452 |              449 |             438 |             438 |
| Discovery full selection, including two additions |         87 |               77 |              77 |              77 |

All 1,617 runtime cells are fresh: 1,611 preservation cells plus six additions. Every previous
classification, return code, execution count, diagnostic and canonical shape is identical to the
accepted slice-204 full checkpoint, with zero missing, extra or duplicate keys. All 1,550 previous
correct cells remain correct, and all 61 known failures retain their recorded behavior. The six
new cells raise the correct total to 1,556. No previous discovery contract changed and neither new
source overlaps frozen v1. This checkpoint has no inherited runtime outcomes. Its source/toolchain
baseline identity audit justified reusing accepted204 evidence instead of repeating the full before
run. Parent acceptance of this full checkpoint resets implementation cadence to zero.

Tested base is `dd30f64a7672f345c3f47a0c1b434d5cb3debb28`, with exact changed source and binary hashes
in `runtime-validation.slice-205.json`. Compiler SHA256 is
`6aac36c1b7068c9f54e537a9c373a267151e902bc42908784ab1fac3a4c95892`; unchanged provider SHA256 is
`1f3ef9bd03de64838dc039a97ec30f4fe00cd33682d0d95b06abac895446124f`.
The manifest rechecked all recorded source/tool hashes after every gate and the two fixture hashes
against their pre-change runs. The L4 stayed healthy. Raw evidence is retained under
`build/nvvm-loop/slice-205-before` and `build/nvvm-loop/slice-205-after`.

All six complex support cells were reassessed. Both NVRTC entries still compile/assemble with
byte-identical PTX. Both direct `sample_buffer` cells now compile/assemble at O0/O3. Both direct
`eval_buffer` cells reject `sequential element pointer: Ptr<bool, addressSpace=2147483647, access=0,
operands=4, layout=ScalarLayout>`. The minimal matching source path is its vector
`any(isnan(eval)) || any(isinf(eval))` check. Inherited slice203 final IR corroborates vector predicate
helpers that store Bool lanes through this exact `getElementPtr` shape. That excerpt is explicitly
historical corroboration, not a fresh attribution of the first failing instruction. No next-feature
investigation or implementation is included. The material is unchanged; missing bindings and
texture/LUT/material/input fixtures and output oracles still prevent material runtime or performance
claims, including for the newly compiling sample entry.
