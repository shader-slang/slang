//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

// Regression test for shader-slang/slang#12563: a non-returned held-pointer conflict inside a
// helper that address-space specialization clones must be diagnosed EXACTLY ONCE, not once per
// clone. `useConflicted` takes a pointer parameter and is called with pointers in two different
// concrete classes (`&buf[0]` StorageBuffer, `&gs` Workgroup), so the pass specializes it into two
// clones. Its local `p` is written a Workgroup pointer (`&gs`) on one path and a StorageBuffer
// pointer (`&buf[3]`) on the other and is only dereferenced, so it is the general inconsistent-slot
// conflict (E58003). That conflict is present in every clone (and the removed original), so before
// E58003 was deferred past dead-clone removal and deduped by specialization root each copy emitted
// its own diagnostic. Exhaustive matching (no `non-exhaustive`) fails if a second E58003 slips
// through.

RWStructuredBuffer<int> buf;

groupshared int gs;

[noinline]
void useConflicted(int* q)
{
    buf[1] = *q; // touch q so the helper is specialized on q's address space (forces clones)
    int* p;
    if (buf[2] == 0)
        p = &gs; // Workgroup
    else
        p = &buf[3]; // StorageBuffer
    buf[0] = *p; // dereferenced, not returned
}
//CHECK: E58003
//CHECK: use of pointer with inconsistent address space

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    gs = 1;
    useConflicted(&buf[0]); // q : StorageBuffer
    useConflicted(&gs); // q : Workgroup
}
