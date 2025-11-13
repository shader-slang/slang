//TEST(compute):SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST(compute):SIMPLE(filecheck=SPIRV_BL):-target spirv-asm -entry computeMain -stage compute -DBLOCK_LOAD

// Requires VK_NV_cooperative_matrix2 extension (cooperativeMatrixTensorAddressing, cooperativeMatrixBlockLoads)
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix -Xslang -DRW

// Requires VK_NV_cooperative_matrix2 extension (cooperativeMatrixTensorAddressing, cooperativeMatrixBlockLoads)
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BL):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix -Xslang -DBLOCK_LOAD
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BL):-vk -output-using-type -emit-spirv-directly -render-feature cooperative-matrix -Xslang -DBLOCK_LOAD -Xslang -DRW

//CHECK: 0
//CHECK-NEXT: 0
//CHECK-NEXT: 0
//CHECK-NEXT: 0
//CHECK-NEXT: 5
//CHECK-NEXT: 6
//CHECK-NEXT: 0
//CHECK-NEXT: 0
//CHECK-NEXT: 9

//CHECK_BL: 0
//CHECK_BL-NEXT: 0
//CHECK_BL-NEXT: 0
//CHECK_BL-NEXT: 0
//CHECK_BL-NEXT: 7
//CHECK_BL-NEXT: C
//CHECK_BL-NEXT: 0
//CHECK_BL-NEXT: 0
//CHECK_BL-NEXT: C
//CHECK_BL-NEXT: 11
//CHECK_BL-NEXT: 0
//CHECK_BL-NEXT: 0

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0 24.0], stride=4, count=256),name=buf

#if defined(RW)
    RWByteAddressBuffer inputBuffer;
#else // #if defined(RW)
    ByteAddressBuffer inputBuffer;
#endif // #else // #if defined(RW)

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<float16_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

float16_t decodeFunc(uint32_t* encoded, uint32_t blockCoord[2], uint32_t coordInBlock[2])
{
    uint32_t coord = blockCoord[1] * 4 + blockCoord[0];
    uint32_t mask = (0xff << (coordInBlock[0] * 8));
    return float16_t(encoded[coord] & mask) + 1.0;
}

[numthreads(32, 1, 1)]
void computeMain()
{
    //SPIRV: = OpCreateTensorLayoutNV %
    TensorLayout<2, CoopMatClampMode.Undefined> tl;

    //SPIRV: = OpTensorLayoutSetDimensionNV %
    let tl1 = tl.Dimension(32, 16);

    //SPIRV: = OpTensorLayoutSetStrideNV %
    let tl2 = tl1.Stride(4, 1);

    //SPIRV: = OpTensorLayoutSliceNV %
    let tl3 = tl2.Slice(4, 24, 0, 16);

    //SPIRV: = OpTensorLayoutSetClampValueNV %
    let tl4 = tl3.ClampValue(CoopMatClampMode.Repeat);

    //SPIRV: = OpTensorLayoutSetBlockSizeNV %
    let tl5 = tl4.BlockSize(4, 8);

#if defined(BLOCK_LOAD)
    //SPIRV_BL: = OpCooperativeMatrixLoadTensorNV %{{.*}} DecodeFunc %
    let mat = CoopMatType.Load<uint32_t>(inputBuffer, 0, tl5, decodeFunc);

#else // #if defined(BLOCK_LOAD)
    //SPIRV: = OpCooperativeMatrixLoadTensorNV %{{.*}} None
    let mat = CoopMatType.Load(inputBuffer, 0, tl5);

#endif // #else // #if defined(BLOCK_LOAD)

    //SPIRV:OpCooperativeMatrixStoreTensorNV %{{.*}} None
    mat.Store(outputBuffer, 0, tl5);
}
