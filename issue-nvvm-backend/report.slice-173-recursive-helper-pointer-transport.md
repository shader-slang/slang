# Slice 173: Recursive helper-pointer transport

## Motivation

Two real dynamic-dispatch workloads exposed adjacent omissions in the helper ABI. The frozen
workload passes interface values through direct returns, `out`, and `inout`:

```slang
void createOut(out IFoo result);
void modify(inout IFoo item);

IFoo result;
factory.createOut(result);
factory.modify(result);
```

Dynamic-dispatch lowering already represented `IFoo` as a finite copyable tuple containing its
tag and AnyValue payload. Direct NVVM nevertheless stopped at `device scalar pointer:
producer=param, consumer=call` when that tuple crossed an output helper parameter.

The discovery workload added one pointer level:

```slang
int dispatchViaDoublePtr(IFoo** pp)
{
    return (*pp)->getVal();
}

IFoo* localPtr = p;
outputBuffer[0] = dispatchViaDoublePtr(__getAddress(localPtr));
```

Its canonical helper parameter was `Ptr<Ptr<Tuple, UserPointer>, UserPointer>`. The existing
helper-value algebra accepted the inner user pointer but stopped at the outer pointer.

## Proposed solution

Make canonical CUDA user pointers recursive helper-value leaves. The exact four-operand,
read-write, `UserPointer`, `DefaultLayout` spelling is supported only when its complete pointee is
already a finite helper value; an active-type set rejects cyclic graphs. Keep copyable values,
aggregate storage, and entry-point ABI admission separate.

Complete pointer validation by admitting the already-supported generic local copyable pointer
family, including `OutParam<Tuple>` and `BorrowInOutParam<Tuple>`, instead of repeating only its
numeric and array subsets. Prove local helper layout through the same recursive struct/array/pointer
algebra used by type lowering. Existing revision-32 typed pointer, load, store, call, aggregate,
and address-space operations express the whole representation, so the provider ABI does not
change.

## Change summary

- Helper type classification recursively admits exact finite CUDA user pointers and lowers nested
  pointees through the generic helper representation.
- Pointer and call validation share the complete local-copyable/local-helper/device-helper
  families; local layout validation accepts the pointer leaf already owned by that algebra.
- The frozen interface return/out/inout workload and discovery double-indirection workload gain
  permanent O0/O3 direct-NVVM differential lanes.
- The measurement harness accepts optional explicit compiler arguments so standalone dynamic-
  dispatch gates can carry the same type conformances as their test inputs.
- Frozen/discovery TSV and Pareto JSON, a two-gate measurement manifest, design documentation,
  the capability ledger, plan, and this report retain the evidence.

## Concepts and vocabulary

**Helper value** is the finite value algebra that may cross a direct helper signature. It includes
copyable scalar/aggregate values and selected pointer/resource leaves but is deliberately broader
than byte-copyable storage.

**CUDA user pointer** is the canonical specialized `Ptr<T, ReadWrite, UserPointer,
DefaultLayout>` spelling produced for Slang `Ptr<..., AddressSpace::Device>`.

**Local helper pointer** is the generic pointer produced by a local `var`, `out`/`inout`
parameter, or `__getAddress`; its producer proves that the storage is local even when its pointee
matches a device helper parameter.

## Process report

The frozen trace begins in dynamic-dispatch lowering. Interface specialization builds
`Tuple { uint tag; AnyValue4 payload; }`; wrapper functions receive exact `OutParam<Tuple>` and
`BorrowInOutParam<Tuple>` parameters, and direct calls pass those parameters onward. Both types
already satisfy `asNVVMSupportedLocalCopyableValuePointerType`. `_isSupportedNVVMHelperArgument`
therefore accepted the call relation, but `_validatePointerValue` reconstructed a smaller list:
numeric locals, local arrays, and non-copyable helper locals. Removing the new complete local-
copyable branch reproduces the original `producer=param, consumer=call` failure. The retained fix
reuses the existing canonical classifier; it does not add an existential or tuple special case.

The discovery trace is produced by the same interface lowering plus CUDA address-space
specialization. Global `IFoo* p` becomes `Ptr<Tuple, UserPointer, DefaultLayout>`. The helper's
`IFoo**` becomes `Ptr<Ptr<Tuple, UserPointer, DefaultLayout>, UserPointer, DefaultLayout>`, while
`__getAddress(localPtr)` is the exact one-operand generic local pointer to the inner type. The old
`_isNVVMSupportedHelperValueType` treated a device pointer as a terminal leaf only when its pointee
was copyable, so it rejected the outer parameter before provider discovery.

`_asNVVMSupportedDeviceHelperValuePointerType` now checks the complete producer spelling and
recurses into the pointee with the same active-type set used for arrays and structs. A pointer to
the copyable existential tuple remains the established inner case; a pointer to that pointer is
now valid for the same reason. A self-referential pointee re-enters the active set and remains
unsupported. The public copyable-pointer classifier is unchanged, so parameter-group storage and
entry-point launch roles do not acquire nested pointers from this helper-only widening.

Type lowering sends a non-copyable nested pointer pointee through `NVVMTypeUse::HelperValue` and
uses LLVM generic address space for the helper signature. `_isSupportedNVVMHelperArgument`
relates the exact one-operand local pointer to the exact device helper parameter only when their
pointee types are equal. `_validatePointerValue` then validates both the function parameter and
the local producer through the same classifiers. The local layout gate formerly required every
non-copyable helper local to be a struct; it now calls the existing recursive helper-layout proof,
whose pointer leaf is fixed-size and whose aggregate branches continue checking CUDA/LLVM sizes,
field offsets, and array strides.

The self-review inventory contains three retained changes. The local-copyable validation branch
survives because removing it reproduces the frozen failure and the exact `OutParam<Tuple>` producer
already owns the supported type. The recursive device-helper classifier survives because removing
it restores the discovery signature failure; it requires the complete canonical spelling and
finite pointee rather than a fixture or syntax name. The generalized local layout call survives
because removing it advances the same discovery workload only to `local helper-value layout`; it
uses the existing recursive representation proof and adds no offset fallback. No syntax
reconstruction, arbitrary operand-graph search, compatibility path, downstream IR patch, or
provider callback was added.

Frozen corpus v1 remains exactly 452 workloads and 427 healthy MVP references. Healthy
correctness advances from 412/412/412 to 413/413/413 O0/O3/both, with exactly the selected frozen
gain and zero old-correct regression. All-row direct totals become 427 correct, four runtime
mismatches, and 21 preflight failures in each mode. Discovery remains exactly 82 workloads and 72
healthy references and advances from 69/69/69 to 70/70/70, again with exactly one gain and no
loss.

The selected regression prefix passes 433/433 and the permanent `nvvm` category passes 76/76.
Both representative gates compile and assemble through CUDA 12.9 for native NVRTC, direct O0
SM70, and direct O3 SM70/SM80/SM90. At SM70, direct O3 PTX is 3,235 bytes versus 10,378 native for
the return/out/inout workload and 537 bytes versus 8,510 native for double indirection. Median
standalone compile times were 261.0 ms versus 356.2 ms and 262.9 ms versus 364.7 ms respectively;
these remain exploratory measurements rather than a controlled benchmark.
