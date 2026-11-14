// Regression test for https://github.com/shader-slang/slang/issues/14398
//
// The core module declares `vector * scalar`, `scalar * vector` and the matrix equivalents as one
// instruction with a scalar operand and a vector operand. The C++ and CUDA preludes only define
// the operators for two operands of the same type, so `simplifyForEmit` splats the scalar. This
// test checks the same splat for the initializer of a module-scope `static const`: it is a tree of
// module-scope instructions, with no function around it.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP): -target cpp -line-directive-mode none -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

// Each scalar operand is splat to the type of the other operand.
// CUDA: make_float3 (1.0f, 2.0f, 3.0f) * make_float3 (2.0f)
// CUDA: make_float3 (3.0f) * make_float3 (1.0f, 2.0f, 3.0f)
// CUDA: makeMatrix<float, 2, 2> (1.0f, 2.0f, 3.0f, 4.0f) * makeMatrix<float, 2, 2> (2.0f)
// CPP: Vector<float, 3> (1.0f, 2.0f, 3.0f) * (Vector<float, 3> )2.0f
// CPP: (Vector<float, 3> )3.0f * Vector<float, 3> (1.0f, 2.0f, 3.0f)
// CPP: Matrix<float, 2, 2> (1.0f, 2.0f, 3.0f, 4.0f) * Matrix<float, 2, 2> (2.0f, 2.0f, 2.0f, 2.0f)

// NVRTC accepts the emitted CUDA source.
// PTX: .entry computeMain

// A vector times a scalar.
static const float3 kVectorTimesScalar = float3(1.0, 2.0, 3.0) * 2.0;
// A scalar times a vector, and a scalar operand inside an array element.
static const float3 kScalarTimesVector = 3.0 * float3(1.0, 2.0, 3.0);
static const float3 kArray[2] = {float3(1.0, 2.0, 3.0) * 2.0, float3(4.0) + 1.0};
// A matrix times a scalar.
static const float2x2 kMatrix = float2x2(1, 2, 3, 4) * 2.0;
// A comparison has a `bool` vector as its result, but still splats the scalar.
static const bool2 kCompare = float2(1.0, 2.0) < 1.5;
// A shift splats the scalar to a vector of its own type.
static const int3 kShift = 8 >> int3(1, 2, 3);

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = kVectorTimesScalar.y;
    outputBuffer[1] = kScalarTimesVector.z;
    outputBuffer[2] = kArray[0].z;
    outputBuffer[3] = kArray[1].x;
    outputBuffer[4] = kMatrix[1][1];
    outputBuffer[5] = kCompare.x ? 1.0 : 0.0;
    outputBuffer[6] = kCompare.y ? 1.0 : 0.0;
    outputBuffer[7] = float(kShift.y);
}

// CHECK: 4.0
// CHECK-NEXT: 9.0
// CHECK-NEXT: 6.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 8.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 2.0
