//TEST(compute):SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute -skip-spirv-validation
//TEST(compute):SIMPLE(filecheck=SPIRV_BL):-target spirv-asm -entry computeMain -stage compute -skip-spirv-validation -DBLOCK_LOAD

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -skip-spirv-validation -render-feature cooperative-matrix-tensor-addressing
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -skip-spirv-validation -render-feature cooperative-matrix-tensor-addressing -Xslang -DRW

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BL):-vk -output-using-type -emit-spirv-directly -Xslang -DBLOCK_LOAD -render-feature cooperative-matrix-block-loads -skip-spirv-validation
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BL):-vk -output-using-type -emit-spirv-directly -Xslang -DBLOCK_LOAD -render-feature cooperative-matrix-block-loads -skip-spirv-validation -Xslang -DRW

//CHECK: 2
//CHECK-NEXT: 2
//CHECK-NEXT: 2
//CHECK-NEXT: 2
//CHECK-NEXT: 12
//CHECK-NEXT: 12
//CHECK-NEXT: 12
//CHECK-NEXT: 12
//CHECK-NEXT: 0

//CHECK_BL: 7
//CHECK_BL-NEXT: 1
//CHECK_BL-NEXT: 1
//CHECK_BL-NEXT: 1
//CHECK_BL-NEXT: 18
//CHECK_BL-NEXT: 1
//CHECK_BL-NEXT: 1
//CHECK_BL-NEXT: 1
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
    TensorLayout<2, CoopMatClampMode.Undefined> tl;

    let tl1 = tl.Dimension(16, 16);
    let tl2 = tl1.Slice(0, 16, 0, 16);
    let tl3 = tl2.BlockSize(4, 1);

    //SPIRV: = OpTypeTensorViewNV %{{[^%]*}} %false %{{[^%]*}} %{{[^%]*$}}
    //SPIRV: = OpCreateTensorViewNV %
    TensorView<2, false, 0, 1> tvRowMajor;
    TensorView<2, false, 1, 0> tvColumnMajor;

    //SPIRV: = OpTensorViewSetDimensionNV %
    let tvColumnMajor1 = tvColumnMajor.Dimension(16, 8);

    //SPIRV: = OpTensorViewSetStrideNV %
    let tvColumnMajor2 = tvColumnMajor1.Stride(8, 1);

    //SPIRV: = OpTensorViewSetClipNV %
    let tvColumnMajor3 = tvColumnMajor2.Clip(0, 8, 0, 64);

#if defined(BLOCK_LOAD)
    //SPIRV_BL: = OpCooperativeMatrixLoadTensorNV %{{.*}} TensorView|DecodeFunc %
    let mat = CoopMatType.Load<uint32_t>(inputBuffer, 0, tl3, tvRowMajor, decodeFunc);

#else // #if defined(BLOCK_LOAD)
    //SPIRV: OpCooperativeMatrixLoadTensorNV %
    //SPIRV-SAME: TensorView
    let mat = CoopMatType.Load(inputBuffer, 0, tl3, tvRowMajor);

#endif // #else // #if defined(BLOCK_LOAD)

    //SPIRV: OpCooperativeMatrixStoreTensorNV {{.*}} TensorView %
    mat.Store(outputBuffer, 0, tl3, tvColumnMajor3);
}
