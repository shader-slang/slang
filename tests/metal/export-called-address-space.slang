//TEST:SIMPLE(filecheck=BUMP): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=READ): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=MIX): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=MIXCOUNT): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=CLONE): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=ORDER): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=ORIG): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry cmain -stage compute

// Exported functions are given `thread` parameters on Metal even when nothing calls them. An
// export that *is* called is still specialized from its call sites, and the original is removed
// once every call targets a specialization, so no unused `thread` copy is emitted next to it.

groupshared int gs;
groupshared int gs2;
RWStructuredBuffer<int> buf;

// A call with a `threadgroup` argument replaces the only use of `bumpGroupShared`, so exactly one
// copy, the `threadgroup` one, is emitted.
// BUMP-NOT: void bumpGroupShared_{{[0-9]+}}(
// BUMP: void bumpGroupShared_{{[0-9]+}}(int threadgroup* {{[A-Za-z0-9_]+}})
// BUMP-NOT: void bumpGroupShared_{{[0-9]+}}(
export void bumpGroupShared(inout int x)
{
    x = x + 1;
}

// The buffer parameter already has the `device` address space and the `out` argument is a local,
// so exactly one copy is emitted, with the same signature as the uncalled export would have.
// READ-NOT: void readBuffer_{{[0-9]+}}(
// READ: void readBuffer_{{[0-9]+}}(int device* {{[A-Za-z0-9_]+}}, int thread* {{[A-Za-z0-9_]+}})
// READ-NOT: void readBuffer_{{[0-9]+}}(
export void readBuffer(RWStructuredBuffer<int> b, out int r)
{
    r = b[0];
}

// A `thread` call and a `threadgroup` call each need their own copy, so exactly two are emitted.
// MIX-DAG: void bumpBoth_{{[0-9]+}}(int thread* {{[A-Za-z0-9_]+}})
// MIX-DAG: void bumpBoth_{{[0-9]+}}(int threadgroup* {{[A-Za-z0-9_]+}})
// MIXCOUNT-COUNT-2: void bumpBoth_{{[0-9]+}}(
// MIXCOUNT-NOT: void bumpBoth_{{[0-9]+}}(
export void bumpBoth(inout int x)
{
    x = x * 3;
}

// Passing a buffer element makes an earlier pass clone `addTo`, and the clone keeps the export
// decoration. Its call is then specialized to `threadgroup`, so the clone itself has no uses left
// and no `thread` variant of it is emitted. The original `addTo` is no longer called after the
// earlier pass, so, like any uncalled export, it is emitted with `thread` parameters (ORIG).
// ORIG: void addTo_{{[0-9]+}}(int thread* {{[A-Za-z0-9_]+}}, int thread* {{[A-Za-z0-9_]+}})
// CLONE-NOT: void addTo_{{[0-9]+}}(int {{[A-Za-z0-9_]+}}, int thread*
// CLONE: void addTo_{{[0-9]+}}(int {{[A-Za-z0-9_]+}}, int threadgroup*
// CLONE-NOT: void addTo_{{[0-9]+}}(int {{[A-Za-z0-9_]+}}, int thread*
export void addTo(inout int a, inout int b)
{
    a += b;
}

// `chainG` calls `chainF` with its own `thread` parameter, so it reuses the seeded `chainF`.
// Both are also called with `threadgroup` arguments, which leaves the seeded originals with no
// uses once `chainG` is removed. Neither original is emitted, whichever call comes first.
// `chainG3` reaches `chainF3` through the non-exported `chainH3`: processing the seeded `chainG3`
// makes a `thread` clone of `chainH3`, which is the last user of the seeded `chainF3` once
// `chainG3` is removed, so that clone is removed too.
// The seeded `chainG4` calls `chainF4` with a `thread` and a `threadgroup` argument, which makes a
// clone of `chainF4`. That clone has no caller once `chainG4` is removed, so it is removed as well.
// ORDER-NOT: void chain{{[FGH][1-4]}}_{{[0-9]+}}(int thread*
// ORDER: void chainF3_{{[0-9]+}}(int threadgroup*
// ORDER-NOT: void chain{{[FGH][1-4]}}_{{[0-9]+}}(int thread*
export void chainF1(inout int x) { x += 1; }
export void chainG1(inout int y) { chainF1(y); }
export void chainF2(inout int x) { x += 2; }
export void chainG2(inout int y) { chainF2(y); }
export void chainF3(inout int x) { x += 3; }
void chainH3(inout int z) { chainF3(z); }
export void chainG3(inout int y) { chainH3(y); }
export void chainF4(inout int a, inout int b) { a += b; }
export void chainG4(inout int y) { chainF4(y, gs); }

// METALLIB: @cmain
[shader("compute")]
[numthreads(1, 1, 1)]
void cmain()
{
    bumpGroupShared(gs);

    int r;
    readBuffer(buf, r);

    int local = r;
    bumpBoth(local);
    bumpBoth(gs2);

    addTo(buf[0], gs);

    chainF1(gs);
    chainG1(gs);
    chainG2(gs2);
    chainF2(gs2);
    chainG3(gs);
    chainG4(gs2);
    buf[1] = local + gs + gs2;
}
