//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

// Regression test for shader-slang/slang#12563 (found while reviewing the fix
// for shader-slang/slang#12498).
//
// `pick` returns pointers of two DIFFERENT storage classes that are each
// individually returnable in SPIR-V's Logical addressing model: the
// `groupshared` global `gShared` ('Workgroup') on one path and a
// `RWStructuredBuffer` element ('StorageBuffer') on the other. A function has a
// single result type, so returning pointers in two classes is target-invalid.
//
// This is the whitelist-internal companion to
// conflicting-return-addrspace-12563.slang: there the first return is a
// function-local ('Function') pointer, so the function is rejected because its
// result class is not returnable at all (E58004). Here both classes ARE
// returnable, so E58004 does not apply; the error is specifically that the two
// returns disagree (E58005) — a distinct condition.
//
// The address-space specialization pass derives the result address space from
// the first return in iteration order and then skips later returns, so it
// converges instead of flipping the result type between Workgroup and
// StorageBuffer on every fixpoint drain and requeuing the function forever
// (without that deterministic choice this compile hangs). The mismatch the
// choice leaves behind — one result type, two return classes — is then
// diagnosed in diagnoseUnreturnableStorageClassPointerReturns rather than
// reaching emission as invalid SPIR-V. That this DIAGNOSTIC_TEST runs to
// completion instead of timing out is the termination check.

RWStructuredBuffer<int> buf;

groupshared int gShared;

[noinline]
int* pick(bool c, uint i)
{
    if (c)
        return &gShared;
    return &buf[i];
}
//CHECK: 58005
//CHECK: more than one storage class

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    gShared = 1;
    int* r = pick(tid.x == 0, tid.x);
    buf[0] = *r;
}
