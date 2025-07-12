//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// `OpSpecConstantOp` can only contain integer operations when targeting Vulkan SPIRV, not floating-point operations.
// This test checks that floating-point operations that strictly contain specialization constant variables are not declared with `OpSpecContantOp`,
// while integer operations that strictly contain specializaton constant operands are declared as `OpSpecConstantOp`.

// CHECK-DAG: OpSpecConstant %float 1
// CHECK-DAG: OpSpecConstant %ulong 256
// CHECK-DAG: OpSpecConstant %float 100
// CHECK-DAG: OpSpecConstantOp %half FConvert
// CHECK-DAG: OpSpecConstantOp %int UConvert

// CHECK-NOT: OpSpecConstantOp {{.*}} FAdd
// CHECK-NOT: OpSpecConstantOp {{.*}} FSub
// CHECK-NOT: OpSpecConstantOp {{.*}} FMul
// CHECK-NOT: OpSpecConstantOp {{.*}} FDiv
// CHECK-NOT: OpSpecConstantOp {{.*}} SpvOpConvertUToF
// CHECK-NOT: OpSpecConstantOp {{.*}} SpvOpConvertFToU

[[SpecializationConstant]]
const float X = 1.0;
[[SpecializationConstant]]
const uint64_t Y = 256;
[[SpecializationConstant]]
const float Z = 100.0;

int func1()
{
    // Test float-to-float and int-to-int conversions.
    int a = int(Y);
    half b = half(X);
    int16_t c = int16_t(Y);

    // Test comparisons.
    if (X < 2.0)
    {
        a = 3;
    }
    else if (X > 5.0)
    {
        a = 5;
    }

    if (Y < 200)
    {
        b = 2.0h;
    }
    else if (Y > 500)
    {
        b = 5.0h;
    }

    return a + int(b) + int(c);
}

float func2()
{
    // Test floating-point arithmetic.
    float a = X + Z;
    a += (X - Z);
    a += (X * Z);
    a += (X / Z);

    return a;
}

float func3()
{
    // Test float-to-int and int-to-float conversions.
    int a = int(Z) * 2;
    return float(Y) + float(a);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // BUF: 818.01
    outputBuffer[0] = float(func1()) + func2() + func3();
}
