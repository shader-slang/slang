//TEST(compute):SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute -skip-spirv-validation
//TEST(compute):SIMPLE(filecheck=SPIRV_BL):-target spirv-asm -entry computeMain -stage compute -skip-spirv-validation -DBLOCK_LOAD

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -skip-spirv-validation -render-feature cooperative-matrix-tensor-addressing
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -skip-spirv-validation -render-feature cooperative-matrix-tensor-addressing -Xslang -DRW

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BL):-vk -output-using-type -emit-spirv-directly -Xslang -DBLOCK_LOAD -skip-spirv-validation -render-feature cooperative-matrix-block-loads
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BL):-vk -output-using-type -emit-spirv-directly -Xslang -DBLOCK_LOAD -skip-spirv-validation -render-feature cooperative-matrix-block-loads -Xslang -DRW

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

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24], stride=4, count=256),name=buf

#if defined(RW)
    RWByteAddressBuffer inputBuffer;
#else // #if defined(RW)
    ByteAddressBuffer inputBuffer;
#endif // #else // #if defined(RW)

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

using namespace tensor_addressing;
typealias CoopMatType = CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

int32_t decodeFunc(uint32_t* encoded, uint32_t blockCoord[2], uint32_t coordInBlock[2])
{
    uint32_t coord = blockCoord[1] * 4 + blockCoord[0];
    uint32_t mask = (0xff << (coordInBlock[0] * 8));
    return int32_t(encoded[coord] & mask) + 1;
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
    let mat = CoopMatType.LoadDecode<uint32_t>(inputBuffer, 0, tl5, decodeFunc);

#else // #if defined(BLOCK_LOAD)
    //SPIRV: = OpCooperativeMatrixLoadTensorNV %{{.*}} None
    let mat = CoopMatType.Load(inputBuffer, 0, tl5);

#endif // #else // #if defined(BLOCK_LOAD)

    //SPIRV:OpCooperativeMatrixStoreTensorNV %{{.*}} None
    mat.Store(outputBuffer, 0, tl5);
}
