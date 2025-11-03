//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// Ensure SPIRV does not do coherent operations here
// SPIRV-NOT: MakePointerAvailable
// SPIRV-NOT: MakePointerVisible


// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4, count=256):name=inputBuffer
uniform int32_t* inputBuffer;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
uniform int32_t* outputBuffer;

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain()
{
    int32_t* ptrIn = inputBuffer;
    int32_t* ptrOut = outputBuffer;

    let stride = 16;
    let mat = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(ptrIn, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(ptrOut, 0, 16);
}
