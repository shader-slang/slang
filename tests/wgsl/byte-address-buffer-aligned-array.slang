// Aligned `(RW)ByteAddressBuffer` loads and stores of arrays on WGSL and Metal (issue #13805).
//
// WGSL and Metal see a byte-address buffer as an array of 32-bit words, so an array access must be
// split into per-element accesses even when its offset is known to be aligned to the whole array.
// Each statement copies one value from `src` to the same offset in `dst`, so `dst` must end up
// equal to `src`.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL_BAD):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL_BAD):-target metal -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -output-using-type
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-llvm -compute -output-using-type

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0 24.0 25.0 26.0 27.0 28.0 29.0 30.0 31.0 32.0 33.0 34.0], stride=4):name=src
ByteAddressBuffer src;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=dst
RWByteAddressBuffer dst;

struct WithArrayField
{
    float4 x;
    float4 b[2];
};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Constant offsets that are multiples of the access size.

    // WGSL: src_{{[0-9]+}}[(u32(0))/4]
    // WGSL: src_{{[0-9]+}}[(u32(28))/4]
    // WGSL: dst_{{[0-9]+}}[(u32(0))/4] = bitcast<u32>
    // WGSL: dst_{{[0-9]+}}[(u32(28))/4] = bitcast<u32>
    // METAL: src_{{[0-9]+}}[(0U)>>2]
    // METAL: src_{{[0-9]+}}[(28U)>>2]
    // METAL: dst_{{[0-9]+}}[(0U)>>2] = as_type<uint32_t>
    // METAL: dst_{{[0-9]+}}[(28U)>>2] = as_type<uint32_t>
    dst.Store<float4[2]>(0, src.Load<float4[2]>(0));

    // WGSL: src_{{[0-9]+}}[(u32(32))/4]
    // WGSL: src_{{[0-9]+}}[(u32(36))/4]
    // WGSL: dst_{{[0-9]+}}[(u32(32))/4] = bitcast<u32>
    // WGSL: dst_{{[0-9]+}}[(u32(36))/4] = bitcast<u32>
    // METAL: src_{{[0-9]+}}[(32U)>>2]
    // METAL: src_{{[0-9]+}}[(36U)>>2]
    // METAL: dst_{{[0-9]+}}[(32U)>>2] = as_type<uint32_t>
    // METAL: dst_{{[0-9]+}}[(36U)>>2] = as_type<uint32_t>
    dst.Store<float[2]>(32, src.Load<float[2]>(32));

    // WGSL: [[U40:[A-Za-z0-9_]+]] : u32 = src_{{[0-9]+}}[(u32(40))/4]
    // WGSL: dst_{{[0-9]+}}[(u32(40))/4] = [[U40]];
    // METAL: [[U40:[A-Za-z0-9_]+]] = as_type<uint>({{.*}}src_{{[0-9]+}}[(40U)>>2])
    // METAL: dst_{{[0-9]+}}[(40U)>>2] = as_type<uint32_t>([[U40]]);
    dst.Store<uint[1]>(40, src.Load<uint[1]>(40));

    // The array field `b` is at byte 48 + 16 = 64, a multiple of its 32-byte size.
    // WGSL: src_{{[0-9]+}}[(u32(64))/4]
    // WGSL: src_{{[0-9]+}}[(u32(92))/4]
    // WGSL: dst_{{[0-9]+}}[(u32(64))/4] = bitcast<u32>
    // WGSL: dst_{{[0-9]+}}[(u32(92))/4] = bitcast<u32>
    // METAL: src_{{[0-9]+}}[(64U)>>2]
    // METAL: src_{{[0-9]+}}[(92U)>>2]
    // METAL: dst_{{[0-9]+}}[(64U)>>2] = as_type<uint32_t>
    // METAL: dst_{{[0-9]+}}[(92U)>>2] = as_type<uint32_t>
    dst.Store<WithArrayField>(48, src.Load<WithArrayField>(48));

    // Run-time offsets whose alignment is promised by the caller.
    uint t = tid.x * 128;

    // WGSL: [[T44:[A-Za-z0-9_]+]] : u32 = {{.*}} + u32(44);
    // WGSL: [[U44:[A-Za-z0-9_]+]] : u32 = src_{{[0-9]+}}[([[T44]])/4]
    // WGSL: dst_{{[0-9]+}}[([[T44]])/4] = [[U44]];
    // METAL: [[T44:[A-Za-z0-9_]+]] = {{.*}} + 44U;
    // METAL: [[U44:[A-Za-z0-9_]+]] = as_type<uint>({{.*}}src_{{[0-9]+}}[([[T44]])>>2])
    // METAL: dst_{{[0-9]+}}[([[T44]])>>2] = as_type<uint32_t>([[U44]]);
    dst.StoreAligned<uint[1]>(t + 44, src.LoadAligned<uint[1]>(t + 44));

    // WGSL: [[T96:[A-Za-z0-9_]+]] : u32 = {{.*}} + u32(96);
    // WGSL: src_{{[0-9]+}}[([[T96]])/4]
    // WGSL: [[T96_28:[A-Za-z0-9_]+]] : u32 = [[T96]] + u32(28);
    // WGSL: src_{{[0-9]+}}[([[T96_28]])/4]
    // WGSL: dst_{{[0-9]+}}[([[T96]])/4] = bitcast<u32>
    // WGSL: dst_{{[0-9]+}}[([[T96_28]])/4] = bitcast<u32>
    // METAL: [[T96:[A-Za-z0-9_]+]] = {{.*}} + 96U;
    // METAL: src_{{[0-9]+}}[([[T96]])>>2]
    // METAL: [[T96_28:[A-Za-z0-9_]+]] = [[T96]] + 28U;
    // METAL: src_{{[0-9]+}}[([[T96_28]])>>2]
    // METAL: dst_{{[0-9]+}}[([[T96]])>>2] = as_type<uint32_t>
    // METAL: dst_{{[0-9]+}}[([[T96_28]])>>2] = as_type<uint32_t>
    dst.Store<float4[2]>(t + 96, src.LoadAligned<float4[2]>(t + 96, 32), 32);

    // WGSL: [[T128:[A-Za-z0-9_]+]] : u32 = {{.*}} + u32(128);
    // WGSL: src_{{[0-9]+}}[([[T128]])/4]
    // WGSL: [[T128_4:[A-Za-z0-9_]+]] : u32 = [[T128]] + u32(4);
    // WGSL: src_{{[0-9]+}}[([[T128_4]])/4]
    // WGSL: dst_{{[0-9]+}}[([[T128]])/4] = bitcast<u32>
    // WGSL: dst_{{[0-9]+}}[([[T128_4]])/4] = bitcast<u32>
    // METAL: [[T128:[A-Za-z0-9_]+]] = {{.*}} + 128U;
    // METAL: src_{{[0-9]+}}[([[T128]])>>2]
    // METAL: [[T128_4:[A-Za-z0-9_]+]] = [[T128]] + 4U;
    // METAL: src_{{[0-9]+}}[([[T128_4]])>>2]
    // METAL: dst_{{[0-9]+}}[([[T128]])>>2] = as_type<uint32_t>
    // METAL: dst_{{[0-9]+}}[([[T128_4]])>>2] = as_type<uint32_t>
    dst.Store<float[2]>(t + 128, src.LoadAligned<float[2]>(t + 128, 8), 8);
}

// Neither target may load or store a whole array as one 32-bit word, or go through 64-bit integers.
// WGSL_BAD-NOT: u64
// WGSL_BAD-NOT: : array<{{.*}}> = src_{{[0-9]+}}[
// METAL_BAD-NOT: as_type<array<
// METAL_BAD-NOT: ulong
// `dst` holds the bits of the floats 1.0 to 34.0, copied from `src`.
// BUF: 3F800000
// BUF-NEXT: 40000000
// BUF-NEXT: 40400000
// BUF-NEXT: 40800000
// BUF-NEXT: 40A00000
// BUF-NEXT: 40C00000
// BUF-NEXT: 40E00000
// BUF-NEXT: 41000000
// BUF-NEXT: 41100000
// BUF-NEXT: 41200000
// BUF-NEXT: 41300000
// BUF-NEXT: 41400000
// BUF-NEXT: 41500000
// BUF-NEXT: 41600000
// BUF-NEXT: 41700000
// BUF-NEXT: 41800000
// BUF-NEXT: 41880000
// BUF-NEXT: 41900000
// BUF-NEXT: 41980000
// BUF-NEXT: 41A00000
// BUF-NEXT: 41A80000
// BUF-NEXT: 41B00000
// BUF-NEXT: 41B80000
// BUF-NEXT: 41C00000
// BUF-NEXT: 41C80000
// BUF-NEXT: 41D00000
// BUF-NEXT: 41D80000
// BUF-NEXT: 41E00000
// BUF-NEXT: 41E80000
// BUF-NEXT: 41F00000
// BUF-NEXT: 41F80000
// BUF-NEXT: 42000000
// BUF-NEXT: 42040000
// BUF-NEXT: 42080000
