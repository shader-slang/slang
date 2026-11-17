// Regression test for https://github.com/shader-slang/slang/issues/13444
//
// A module-scope `static const` vector or matrix whose initializer needs a cast used to reach the
// C++ and CUDA emitters as `floatCast(makeMatrix(<literals>))`, because SCCP folds scalar casts
// but not casts of `makeVector` and `makeMatrix`. Both instructions then became `__device__`
// globals, and NVRTC rejects the cast one: "dynamic initialization is not supported for a
// __device__ variable". The peephole pass now rewrites such a cast as the vector or matrix of the
// destination type built from casted literals, and SCCP folds those scalar casts.
//
// The casts in this test come from three sources: an explicit matrix layout that differs from the
// default one, an integer-to-float conversion, and an array of matrices with a non-default layout.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -entry computeMain -stage compute

// NVRTC accepts the emitted CUDA source.
// PTX: .entry computeMain

// No cast is left in the emitted initializers.
// CUDA-NOT: (float)
// CUDA-NOT: __device__ static const Matrix<int
// CUDA-NOT: __device__ static const int3

static const row_major float2x2 kRowMajor = float2x2(0, 1, 2, 3);
static const column_major float2x2 kColumnMajor = float2x2(4, 5, 6, 7);
static const float3 kVector = float3(int3(1, 2, 3));
static const float2x2 kMatrix = float2x2(int2x2(8, 9, 10, 11));
static const row_major float2x2 kArray[2] = {float2x2(0, 1, 2, 3), float2x2(1, 2, 3, 4)};

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = kRowMajor[1][0];
    outputBuffer[1] = kColumnMajor[0][1];
    outputBuffer[2] = kVector.y;
    outputBuffer[3] = kMatrix[1][1];
    outputBuffer[4] = kArray[int(outputBuffer[7])][1][0];
}

// CHECK: 2.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 11.0
// CHECK-NEXT: 2.0
