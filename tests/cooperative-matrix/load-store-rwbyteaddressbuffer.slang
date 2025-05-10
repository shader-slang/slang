//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -skip-spirv-validation
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -skip-spirv-validation -Xslang -DRWBAB

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4, count=256):name=inputBuffer
#if defined(RWBAB)
RWByteAddressBuffer inputBuffer;
#else
ByteAddressBuffer inputBuffer;
#endif

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBuffer, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, 16);
}
