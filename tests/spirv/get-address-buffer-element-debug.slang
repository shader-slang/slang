//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=CHECKDBG):-target spirv -entry computeMain -stage compute -emit-spirv-directly -g2 -O0

// Regression test for shader-slang/slang#12581. The first run uses -O0 so the pointer local is
// not optimized away; the second adds full debug info (-g2), the shape CI originally caught.
//
// A pointer taken to a mutable structured-buffer element via either __getAddress(buf[i]) or
// &buf[i] is a descriptor-backed, logical StorageBuffer pointer. The surface int* type defaults
// to AddressSpace.Device (a physical pointer), so the pointer local slot used to keep a physical
// pointee even though the value stored into it is a StorageBuffer pointer. A surviving slot (a
// -O0 local, or a -g debug backing variable) then produced a store whose pointer and object
// storage classes disagreed, failing SPIR-V validation. The address-space specialization pass now
// specializes such a slot's contained pointer to the value's StorageBuffer space, so both
// spellings stay logical StorageBuffer end-to-end and validation passes.
//
// The buggy output keeps the *slot's* contained pointee physical (a
// _ptr_Function__ptr_<physical>_int slot with a mismatched OpStore). Asserting the logical
// StorageBuffer pointer is *present* is not enough on its own -- the element value is always a
// StorageBuffer pointer, so it appears in the buggy output too. This file takes no
// integer-to-pointer cast, so a correct compile emits no physical pointer at all; asserting the
// physical storage class is *absent* is what distinguishes the fixed output from the buggy one.
// (slang-test removes -g embedded source before FileCheck for SPIR-V targets, so the CHECK-NOT
// does not self-match this comment.)

// A `CHECK-NOT` only spans the gap to the next positive `CHECK`, so a second `CHECK-NOT` after
// the positive line extends the "no physical pointer anywhere" assertion over the rest of the
// output (a physical storage class emitted later would otherwise slip past).
//CHECK-NOT: PhysicalStorageBuffer
//CHECK: OpTypePointer StorageBuffer %int
//CHECK-NOT: PhysicalStorageBuffer

//CHECKDBG-NOT: PhysicalStorageBuffer
//CHECKDBG: OpTypePointer StorageBuffer %int
//CHECKDBG-NOT: PhysicalStorageBuffer

RWStructuredBuffer<int> outputBuffer;
RWStructuredBuffer<int> dataBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int* p = __getAddress(dataBuffer[0]);
    int* q = &dataBuffer[1];

    // Reassignment across control flow: the slot must still resolve to a single consistent
    // StorageBuffer address space.
    if (tid.x == 0)
        p = q;

    outputBuffer[0] = *p;

    // A derived pointer op on the surviving (non-SSA) slot value must inherit the specialized
    // StorageBuffer address space too.
    outputBuffer[1] = p[2];

    // A pointer *derived* from a surviving slot must also become StorageBuffer. Here a
    // conditional reassignment forces `s` to survive as a slot, and `d` is derived from a load
    // of that slot (`s + 1`). The specialization must resolve the derived pointer through to
    // its base slot; otherwise the debug backing variable for `d` keeps a physical (Device)
    // pointee while the value stored is a StorageBuffer pointer, and the -g2 run fails
    // validation with `OpStore ... type does not match Object` — the same failure class this
    // fix addresses, reached via a derived pointer rather than a direct one.
    int* s = &dataBuffer[2];
    int* alt = &dataBuffer[3];
    if (tid.x == 1)
        s = alt;
    int* d = s + 1;
    outputBuffer[2] = *d;

    // A control-flow-merged pointer stored into a *further* slot must resolve too: `m` is fed
    // by `p` on one path and the derived `d` on the other. The reconcile/propagate fixpoint
    // must carry the StorageBuffer space through the merge into `m`.
    int* m;
    if (tid.x < 2)
        m = p;
    else
        m = d;
    outputBuffer[4] = *m;
}
