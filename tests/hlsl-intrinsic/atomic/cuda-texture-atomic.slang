// Emit + assemble coverage for CUDA/PTX `sured` surface-reduction lowering of
// the result-discarding `InterlockedAdd/Min/Max/And/Or` on an `RWTexture` texel
// (shader-slang/slang#12636). The `-target cuda` run checks that the emitter
// calls the `__slang_surface_reduce_<op>_<ctype>` prelude helpers with
// byte-addressed coordinates; the `-target ptx` run compiles through NVRTC and
// FileCheck verifies the exact `sured.b.<op>.<geom>.<ctype>` opcodes in the
// generated PTX so the lowering cannot silently regress. (Offline `nvcc`
// assembly of every helper — including the signed/64-bit forms that this SIMPLE
// test only checks the emitted text of — is exercised by
// `tests/cuda/cuda-texture-atomic-sured.cu`.)

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute -capability cuda_sm_8_0
//TEST:SIMPLE(filecheck=CHECK_PTX):-target ptx -entry computeMain -stage compute -capability cuda_sm_8_0

// 1D scalar / 2D scalar / 3D scalar textures, plus a 2D vector texel for the
// component-offset path, and 64-bit (unsigned and signed) for add/min/max.
RWTexture1D<uint> tex1D;
RWTexture2D<uint> tex2D;
RWTexture2D<int> tex2Ds;
RWTexture3D<uint> tex3D;
RWTexture2D<uint4> tex2DVec;
RWTexture2D<uint64_t> tex2D64;
RWTexture2D<int64_t> tex2D64s;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // --- add ---
    // 1D: single coordinate operand.
    //CHECK: __slang_surface_reduce_add_u32({{.*}}, ({{.*}}) * 4, 1U)
    //CHECK_PTX: sured.b.add.1d.u32.zero
    InterlockedAdd(tex1D[1u], 1u);

    // 2D: byte-addressed x, texel-indexed y.
    //CHECK: __slang_surface_reduce_add_u32({{.*}}, ({{.*}}.x) * 4, {{.*}}.y, 2U)
    //CHECK_PTX: sured.b.add.2d.u32.zero
    InterlockedAdd(tex2D[uint2(1, 2)], 2u);

    // 3D: x, y, z (plus the dummy 4th coord operand in the helper).
    //CHECK: __slang_surface_reduce_add_u32({{.*}}.x) * 4, {{.*}}.y, {{.*}}.z, 3U)
    //CHECK_PTX: sured.b.add.3d.u32.zero
    InterlockedAdd(tex3D[uint3(1, 2, 3)], 3u);

    // 64-bit add uses the u64 channel type (sign-agnostic).
    //CHECK: __slang_surface_reduce_add_u64({{.*}}.x) * 8
    //CHECK_PTX: sured.b.add.2d.u64.zero
    InterlockedAdd(tex2D64[uint2(1, 2)], 4ull);

    // --- min / max, unsigned and signed, 32- and 64-bit ---
    //CHECK: __slang_surface_reduce_min_u32(
    //CHECK_PTX: sured.b.min.2d.u32.zero
    InterlockedMin(tex2D[uint2(1, 2)], 5u);

    //CHECK: __slang_surface_reduce_min_s32(
    //CHECK_PTX: sured.b.min.2d.s32.zero
    InterlockedMin(tex2Ds[uint2(1, 2)], 6);

    //CHECK: __slang_surface_reduce_min_u64(
    //CHECK_PTX: sured.b.min.2d.u64.zero
    InterlockedMin(tex2D64[uint2(1, 2)], 7ull);

    //CHECK: __slang_surface_reduce_min_s64(
    //CHECK_PTX: sured.b.min.2d.s64.zero
    InterlockedMin(tex2D64s[uint2(1, 2)], 8ll);

    //CHECK: __slang_surface_reduce_max_u32(
    //CHECK_PTX: sured.b.max.2d.u32.zero
    InterlockedMax(tex2D[uint2(1, 2)], 9u);

    //CHECK: __slang_surface_reduce_max_s32(
    //CHECK_PTX: sured.b.max.2d.s32.zero
    InterlockedMax(tex2Ds[uint2(1, 2)], 10);

    //CHECK: __slang_surface_reduce_max_u64(
    //CHECK_PTX: sured.b.max.2d.u64.zero
    InterlockedMax(tex2D64[uint2(1, 2)], 11ull);

    //CHECK: __slang_surface_reduce_max_s64(
    //CHECK_PTX: sured.b.max.2d.s64.zero
    InterlockedMax(tex2D64s[uint2(1, 2)], 12ll);

    // --- and / or (b32) ---
    //CHECK: __slang_surface_reduce_and_b32(
    //CHECK_PTX: sured.b.and.2d.b32.zero
    InterlockedAnd(tex2D[uint2(1, 2)], 0xFu);

    //CHECK: __slang_surface_reduce_or_b32(
    //CHECK_PTX: sured.b.or.2d.b32.zero
    InterlockedOr(tex2D[uint2(1, 2)], 0x10u);

    // --- vector-texel component: `.y` folds a +4 channel byte offset into x ---
    //CHECK: __slang_surface_reduce_add_u32({{.*}}.x) * 16 + 4, {{.*}}.y, 13U)
    //CHECK_PTX: sured.b.add.2d.u32.zero
    InterlockedAdd(tex2DVec[uint2(1, 2)].y, 13u);
}
