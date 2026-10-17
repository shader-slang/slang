//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

// Regression test for shader-slang/slang#12563: two GENUINELY DISTINCT non-returned conflicting
// slots in one function must EACH be diagnosed. The E58003 report dedups by source-slot identity
// (`(specialization root, slot ordinal)`), not by function root, so distinct slots do not collapse
// to one diagnostic. `p` and `q` are each written a Workgroup pointer (`&gs`) on one path and a
// StorageBuffer pointer (`&buf[i]`) on the other and are only dereferenced, so each is an
// independent inconsistent-slot conflict (E58003). `computeMain` has no pointer parameter, so it is
// its own specialization root and is never cloned; both conflicts therefore share one root and
// would collapse to a single diagnostic under function-root dedup. Exhaustive matching (no
// `non-exhaustive`) requires BOTH E58003s — this is the companion to held-ptr-conflict-dedup-12563
// (where clones of ONE slot must collapse to one).

RWStructuredBuffer<int> buf;

groupshared int gs;

//CHECK: E58003
//CHECK: use of pointer with inconsistent address space
//CHECK: E58003
//CHECK: use of pointer with inconsistent address space

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    gs = 1;

    int* p;
    if (tid.x == 0)
        p = &gs; // Workgroup
    else
        p = &buf[1]; // StorageBuffer
    buf[0] = *p; // conflict A: dereferenced, not returned

    int* q;
    if (tid.y == 0)
        q = &gs; // Workgroup
    else
        q = &buf[2]; // StorageBuffer
    buf[3] = *q; // conflict B: dereferenced, not returned
}
