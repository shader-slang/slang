// An atomic whose destination is one element of a vector must not take the address of that
// element, which MSL rejects ("address of vector element requested"). The element is addressed
// through a scalar pointer to the vector's storage instead (#13551).

//TEST:SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOELEMADDR):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HELPER):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=LIB):-target metallib -entry computeMain -stage compute

// No atomic destination takes the address of a vector element (`&v[i]` or `&v.x`), which MSL rejects.
// NOELEMADDR-NOT: {{\(atomic_[a-z]+ [a-z]+\*\)\(&.*(\]|\.[xyzw])\)\)}}

struct Cell
{
    uint4 data;
    int2 a;
    uint3 b;
    uint2x2 m;
};

RWStructuredBuffer<Cell> cells;
RWStructuredBuffer<uint4> vecs;
RWStructuredBuffer<uint> outBuf;
groupshared uint4 gs;

// An element passed by reference to a helper that performs the atomic. `[noRefInline]` keeps the
// helper from being inlined, and because the argument is rooted at a shader parameter, buffer-load
// specialization rewrites the helper to address `cells` itself, so the element pointer is formed
// inside the helper rather than copied through a temporary.
// HELPER: void bump_{{[0-9]+}}(
// HELPER-NOT: {{\[\[kernel\]\]}}
// HELPER: atomic_fetch_add_explicit(((atomic_uint device*)(((uint device*)(&{{.*}}->data_{{[0-9]+}})) + {{.*}})), 1U
[noRefInline]
void bump(__ref uint x)
{
    InterlockedAdd(x, 1u);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    // A `uint4` member of a structured-buffer element (stored as `packed_uint4`), constant index.
    // MTL: atomic_fetch_add_explicit(((atomic_uint device*)(((uint device*)(&{{.*}}->data_{{[0-9]+}})) + int(0))), 1U
    // MTL: atomic_fetch_add_explicit(((atomic_uint device*)(((uint device*)(&{{.*}}->data_{{[0-9]+}})) + int(3))), 4U
    // LIB-DAG: call {{.*}}.atomic.global.add.u.i32
    InterlockedAdd(cells[0].data.x, 1u);
    InterlockedAdd(cells[0].data.w, 4u);

    // MTL: atomic_fetch_add_explicit(((atomic_uint device*)(((uint device*)({{.*}}vecs_{{[0-9]+}}+int(0))) + int(0))), 1U
    // MTL: atomic_fetch_add_explicit(((atomic_uint device*)(((uint device*)({{.*}}vecs_{{[0-9]+}}+{{.*}})) + {{.*}})), 1U
    InterlockedAdd(vecs[0].x, 1u);
    InterlockedAdd(vecs[id.x][id.y & 3], 1u);

    // MTL: atomic_fetch_add_explicit(((atomic_int device*)(((int device*)(&{{.*}}->a_{{[0-9]+}})) + int(1))), int(1)
    // MTL: atomic_fetch_add_explicit(((atomic_uint device*)(((uint device*)(&{{.*}}->b_{{[0-9]+}})) + int(2))), 1U
    // LIB-DAG: call {{.*}}.atomic.global.add.s.i32
    InterlockedAdd(cells[0].a.y, 1);
    InterlockedAdd(cells[0].b.z, 1u);

    // An element of a `groupshared` vector keeps the `threadgroup` address space.
    // MTL: atomic_fetch_add_explicit(((atomic_uint threadgroup*)(((uint threadgroup*)({{.*}}gs_{{[0-9]+}})) + int(1))), 1U
    // LIB-DAG: call {{.*}}.atomic.local.add.u.i32
    InterlockedAdd(gs.y, 1u);

    // MTL: atomic_compare_exchange_weak_explicit(((atomic_uint device*)(((uint device*)(&{{.*}}->data_{{[0-9]+}})) + {{.*}})),
    // MTL: atomic_fetch_max_explicit(((atomic_uint device*)(((uint device*)(&{{.*}}->data_{{[0-9]+}})) + int(2))), 7U
    // MTL: atomic_exchange_explicit(((atomic_uint device*)(((uint device*)(&{{.*}}->data_{{[0-9]+}})) + int(3))), 2U
    // LIB-DAG: call {{.*}}.atomic.global.cmpxchg.weak.i32
    // LIB-DAG: call {{.*}}.atomic.global.max.u.i32
    // LIB-DAG: call {{.*}}.atomic.global.xchg.i32
    uint orig;
    InterlockedCompareExchange(cells[id.x].data[id.y & 3], 0u, 1u, orig);
    InterlockedMax(cells[0].data.z, 7u);
    uint prev;
    InterlockedExchange(cells[0].data.w, 2u, prev);

    // A matrix element: the row is an array element (addressable), the column a vector element.
    // MTL: atomic_fetch_add_explicit(((atomic_uint device*)(((uint device*)(&{{.*}}->data_{{[0-9]+}}[int(1)])) + int(0))), 1U
    InterlockedAdd(cells[0].m[1][0], 1u);

    bump(cells[1].data.y);

    GroupMemoryBarrierWithGroupSync();
    outBuf[id.x] = gs.y + orig + prev;
}
