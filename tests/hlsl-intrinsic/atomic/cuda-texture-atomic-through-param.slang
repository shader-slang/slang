// A texel atomic on an `RWTexture` reached through a *function parameter* still
// lowers to `sured` on CUDA/PTX (shader-slang/slang#12636). The parameter has no
// caller-visible `[format(...)]`, so the emitter uses the element-type byte stride
// (4 bytes for a `uint` texel, 8 for a `uint64_t`); this is correct whenever the
// backing format is consistent with the element type. A *contradictory*-format
// texture reached this way is the pre-existing, systemic surf-read/write
// limitation tracked in #12737 — deliberately not made stricter here. This test
// pins that the ordinary through-parameter case emits rather than regressing to a
// diagnostic.
//
// The helpers are checked in the order the CUDA emitter writes them: the two
// `__device__` helper bodies are emitted before `computeMain`, so their `CHECK`
// lines precede any check of the entry point (there are none here). FileCheck
// matches top-to-bottom without backtracking, so keeping the helpers (and their
// checks) first in source order matches the emitted order.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute -capability cuda_sm_8_0
//TEST:SIMPLE(filecheck=CHECK_PTX):-target ptx -entry computeMain -stage compute -capability cuda_sm_8_0

RWTexture2D<uint> tex32;
RWTexture2D<uint64_t> tex64;

// 32-bit through a parameter: element-type stride 4.
void addThroughParam32(RWTexture2D<uint> t)
{
    //CHECK: __slang_surface_reduce_add_u32({{.*}}) * 4
    InterlockedAdd(t[uint2(1, 2)], 20u);
}

// 64-bit through a parameter: element-type stride 8.
void addThroughParam64(RWTexture2D<uint64_t> t)
{
    //CHECK: __slang_surface_reduce_add_u64({{.*}}) * 8
    InterlockedAdd(t[uint2(1, 2)], 21ull);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // In PTX both helpers are inlined into the entry point, so both opcodes appear
    // here in call order.
    //CHECK_PTX: sured.b.add.2d.u32.zero
    addThroughParam32(tex32);
    //CHECK_PTX: sured.b.add.2d.u64.zero
    addThroughParam64(tex64);
}
