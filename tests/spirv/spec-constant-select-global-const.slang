//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Test that operations on specialization constants used in global constant
// initializers compile to OpSpecConstantOp in SPIR-V.
// Regression test for https://github.com/shader-slang/slang/issues/10373

// CHECK-DAG: %[[FOO2:[0-9A-Za-z_]+]] = OpSpecConstant %uint 3
// CHECK-DAG: %[[FOO1:[0-9A-Za-z_]+]] = OpSpecConstant %uint 12
// CHECK-DAG: %[[ZERO:[0-9A-Za-z_]+]] = OpConstant %uint 0

// select (ternary) with comparison and division
// CHECK-DAG: %[[CMP:[0-9A-Za-z_]+]] = OpSpecConstantOp %bool IEqual %[[FOO2]] %[[ZERO]]
// CHECK-DAG: %[[DIV:[0-9A-Za-z_]+]] = OpSpecConstantOp %uint UDiv %[[FOO1]] %[[FOO2]]
// CHECK-DAG: OpSpecConstantOp %uint Select %[[CMP]] %[[ZERO]] %[[DIV]]

// arithmetic and comparison operators
// CHECK-DAG: OpSpecConstantOp %uint IAdd %[[FOO1]] %[[FOO2]]
// CHECK-DAG: OpSpecConstantOp %uint IMul %[[FOO1]] %[[FOO2]]
// CHECK-DAG: OpSpecConstantOp %bool IEqual %[[FOO1]] %[[FOO2]]

[vk::specialization_constant] const uint FOO1 = 12;
[vk::specialization_constant] const uint FOO2 = 3;

static const uint value = FOO2 == 0 ? 0 : FOO1 / FOO2;
static const uint sum = FOO1 + FOO2;
static const uint product = FOO1 * FOO2;
static const bool isEqual = FOO1 == FOO2;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // FOO1=12, FOO2=3:
    // BUF: 4
    outputBuffer[0] = value;
    // BUF-NEXT: 15
    outputBuffer[1] = sum;
    // BUF-NEXT: 36
    outputBuffer[2] = product;
    // BUF-NEXT: 0
    outputBuffer[3] = uint(isEqual);
}
