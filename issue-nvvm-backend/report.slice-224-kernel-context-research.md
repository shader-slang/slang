# Isolate Boolean state in generated kernel contexts

## Motivation

Two frozen masked-prefix min/max workloads reject a helper parameter with type
`Ptr<KernelContext, addressSpace=1, access=0, operands=4, layout=DefaultLayout>`. That diagnostic does
not establish a malformed pointer. Earlier slice 115 already admitted this producer's ThreadLocal
spelling for selected scalar contexts. This slice identifies the actual rejected pointee and proves
an independent runtime reproduction before proposing admission changes.

## Proposed solution

Research only: inspect the original producer IR, compare a minimal Boolean global with integer global
and local Boolean-struct controls, and measure all three modes on unchanged accepted 223 artifacts.
The next bounded implementation should reuse existing recursive copyable-value support for canonical
ThreadLocal context pointers, preserving exact pointer qualifiers and leaving resource-bearing
explicit contexts outside that slice. It needs a real runtime regression and a full checkpoint.

## Change summary

Completed plan, report, semantic evidence and STATUS. Raw sources, probe, IR dumps, PTX/cubins and
results remain under `build/nvvm-loop/slice-224-context`. No production, provider, ABI, registered
source, oracle or manifest change.

## Concepts and vocabulary

A _KernelContext_ is generated per-invocation storage for plain module globals. The source
_ThreadLocal_ address-space enum has value 1; it must not be confused with LLVM/NVVM global address
space 1. A _copyable-value struct_ is the existing finite scalar/vector/array/struct value algebra,
including Boolean fields; it is broader than the older flat integer/float32 scalar-struct subset.

## Process report

The original prefix-min source declares Boolean `isFirstInPartition` and three uint globals.
After `introduceExplicitGlobalContext`, its context contains those exact four fields. The producer
creates a compact entry-local `Ptr<KernelContext>` variable, initializes it per invocation, and gives
reachable helpers an explicit read-write ThreadLocal/default-layout pointer to the same struct.
`findOrCreateContextPtrForFunc` updates signatures and callers. This intentionally different pointer
spelling was already established and admitted for scalar contexts in slice 115; no producer defect
or global-storage reinterpretation is indicated.

`_isSupportedNVVMHelperParameterType` asks
`asNVVMSupportedLocalResourceStructPointerType` to validate the type. The pointee already satisfies
`asNVVMSupportedResourceStructType`, but the explicit ThreadLocal branch additionally requires
`asNVVMSupportedScalarStructType`. That older classifier accepts integer scalars and Float32 only,
rejecting the valid Bool field. The compact-local and mutable-borrow branches do not impose that
extra scalar-only restriction. The diagnostic therefore reports the whole pointer despite the
pointee classification being the cause.

Consider the minimal source:

```slang
static bool flag = false;
[noinline] uint update(uint value)
{
    uint previous = flag ? 1 : 0;
    flag = (value & 1) != 0;
    return previous;
}
[noinline] uint flip()
{
    flag = !flag;
    return flag ? 1 : 0;
}
```

The kernel loads one runtime value per lane, stores update's old value, reads the updated flag, then
stores flip's result. The independent expected rows are zero, input parity, and inverted parity.
This checks initialization, mutation, helper-to-caller state and a second helper access for 32
independent invocations. Its generated context contains only Bool and reproduces both direct
preflight failures. NVRTC executes all 96 output words correctly. Neither direct mode emits PTX or
executes this Boolean-global case, so research does not claim support.

An otherwise equivalent uint global gives an admitted UInt context; all three modes return the
expected 96 words. A local `struct Flags { bool flag; }` passed through inout helper parameters also
passes every mode, proving existing Boolean struct storage/mutation support without the explicit
ThreadLocal spelling. Its producer trace shows mutable-borrow helper pointers and the compact
local argument. These controls isolate admission rather than assuming an LLVM Boolean layout fix.

Across nine compile cells, seven compile/assemble and execute; all 672 output words match and input
words remain unchanged. The remaining two cells are exact Boolean-global preflight rejections.
GPU smoke 4/4 passes. All 24 tested-source hashes, 12 artifacts and 550 runtime-input hashes match 223.
Original registered failures and full corpus/material outcomes explicitly inherit 223; they are not
rerun or reclassified. Full checkpoint 223 and implementation cadence 0 remain unchanged.

The helper/fallback inventory has no production additions. Research uses the existing CUDA-driver
harness and independent parity expectations; no temporary support patch is retained or tested.
Fresh-context delegation remains unavailable, so parent review follows WORKFLOW's local fallback.

Research is accepted on 2026-09-25. The next slice must prove Boolean and representative recursive
value contexts execute correctly, preserve ordinary local/shared/device pointer contracts, and
complete full preservation review. Reassess the two prefix workloads afterward, recording any next
independent unsupported operation without treating diagnostic advancement as a fixed workload.
