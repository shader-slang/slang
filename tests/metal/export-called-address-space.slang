//TEST:SIMPLE(filecheck=BUMP): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=READ): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=MIX): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=MIXCOUNT): -target metal -entry cmain -stage compute
//TEST:SIMPLE(filecheck=CLONE): -target metal -entry cmain -stage compute

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

// The buffer parameter already has the `device` address space, so the call reuses the exported
// function rather than emitting an identical copy.
// READ-NOT: void readBuffer_{{[0-9]+}}(
// READ: void readBuffer_{{[0-9]+}}(int device* {{[A-Za-z0-9_]+}}, int thread* {{[A-Za-z0-9_]+}})
// READ-NOT: void readBuffer_{{[0-9]+}}(
export void readBuffer(RWStructuredBuffer<int> b, out int r)
{
    r = b[0];
}

// A `thread` call reuses the exported function and a `threadgroup` call specializes a copy of it.
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
// and no `thread` variant of it is emitted.
// CLONE-NOT: void addTo_{{[0-9]+}}(int {{[A-Za-z0-9_]+}}, int thread*
// CLONE: void addTo_{{[0-9]+}}(int {{[A-Za-z0-9_]+}}, int threadgroup*
// CLONE-NOT: void addTo_{{[0-9]+}}(int {{[A-Za-z0-9_]+}}, int thread*
export void addTo(inout int a, inout int b)
{
    a += b;
}

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
    buf[1] = local + gs + gs2;
}
