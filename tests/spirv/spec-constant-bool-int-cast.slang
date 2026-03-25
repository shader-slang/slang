//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Test that bool <-> integer specialization constant casts produce valid SPIR-V.
//
// Bool-to-integer uses OpSelect (UConvert/SConvert cannot accept bool operands).
// Integer-to-bool uses INotEqual (UConvert/SConvert cannot produce bool results).

// CHECK-DAG: %[[BVAL:[0-9A-Za-z_]+]] = OpSpecConstantTrue %bool
// CHECK-DAG: %[[UVAL:[0-9A-Za-z_]+]] = OpSpecConstant %uint 5

// bool -> uint: OpSelect picking 1 or 0
// CHECK-DAG: %[[ONE:[0-9A-Za-z_]+]] = OpConstant %uint 1
// CHECK-DAG: %[[ZERO:[0-9A-Za-z_]+]] = OpConstant %uint 0
// CHECK-DAG: OpSpecConstantOp %uint Select %[[BVAL]] %[[ONE]] %[[ZERO]]

// uint -> bool: INotEqual against 0
// CHECK-DAG: OpSpecConstantOp %bool INotEqual %[[UVAL]] %[[ZERO]]

[SpecializationConstant]
const bool B = true;

[SpecializationConstant]
const uint U = 5;

static const uint boolAsUint = uint(B);
static const bool uintAsBool = bool(U);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // uint(true) = 1
    // BUF: 1
    outputBuffer[0] = boolAsUint;

    // uint(bool(5)) = uint(true) = 1
    // BUF-NEXT: 1
    outputBuffer[1] = uint(uintAsBool);

    // BUF-NEXT: 5
    outputBuffer[2] = U;

    // bool(5) is true, so boolAsUint + U = 1 + 5 = 6
    // BUF-NEXT: 6
    outputBuffer[3] = uintAsBool ? boolAsUint + U : 0;
}
