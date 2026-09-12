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

// -O0 run: no debug info, so the disassembly does not embed this source; a plain error scan is safe.
//CHECK-NOT: error:
//CHECK: OpTypePointer StorageBuffer %int

// -g2 run: -g embeds the source text into an OpString, so a CHECK-NOT would match this very
// comment; assert positively instead and let SPIR-V validation gate correctness.
//CHECKDBG: OpTypePointer StorageBuffer %int

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
}
