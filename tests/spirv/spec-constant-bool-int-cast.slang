//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Test that bool <-> integer specialization constant casts produce valid SPIR-V.
//
// Bool-to-integer uses OpSelect (UConvert/SConvert cannot accept bool operands).
// Integer-to-bool uses INotEqual (UConvert/SConvert cannot produce bool results).

// CHECK-DAG: %[[BVAL:[0-9A-Za-z_]+]] = OpSpecConstantTrue %bool
// CHECK-DAG: %[[UVAL:[0-9A-Za-z_]+]] = OpSpecConstant %uint 5
// CHECK-DAG: %[[IVAL:[0-9A-Za-z_]+]] = OpSpecConstant %int -3

// bool -> uint: OpSelect picking 1 or 0
// CHECK-DAG: %[[UONE:[0-9A-Za-z_]+]] = OpConstant %uint 1
// CHECK-DAG: %[[UZERO:[0-9A-Za-z_]+]] = OpConstant %uint 0
// CHECK-DAG: OpSpecConstantOp %uint Select %[[BVAL]] %[[UONE]] %[[UZERO]]

// bool -> int: OpSelect picking 1 or 0
// CHECK-DAG: %[[IONE:[0-9A-Za-z_]+]] = OpConstant %int 1
// CHECK-DAG: %[[IZERO:[0-9A-Za-z_]+]] = OpConstant %int 0
// CHECK-DAG: OpSpecConstantOp %int Select %[[BVAL]] %[[IONE]] %[[IZERO]]

// uint -> bool: INotEqual against 0
// CHECK-DAG: OpSpecConstantOp %bool INotEqual %[[UVAL]] %[[UZERO]]

// int -> bool: INotEqual against 0
// CHECK-DAG: OpSpecConstantOp %bool INotEqual %[[IVAL]] %[[IZERO]]

[SpecializationConstant]
const bool B = true;

[SpecializationConstant]
const uint U = 5;

[SpecializationConstant]
const int I = -3;

static const uint boolAsUint = uint(B);
static const int boolAsInt = int(B);
static const bool uintAsBool = bool(U);
static const bool intAsBool = bool(I);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // uint(true) = 1
    // BUF: 1
    outputBuffer[0] = boolAsUint;

    // int(true) = 1
    // BUF-NEXT: 1
    outputBuffer[1] = uint(boolAsInt);

    // uint(bool(5)) = uint(true) = 1
    // BUF-NEXT: 1
    outputBuffer[2] = uint(uintAsBool);

    // int(-3) != 0, so bool(-3) = true, uint(true) = 1
    // BUF-NEXT: 1
    outputBuffer[3] = uint(intAsBool);

    // bool(5) is true, so boolAsUint + U = 1 + 5 = 6
    // BUF-NEXT: 6
    outputBuffer[4] = uintAsBool ? boolAsUint + U : 0;

    // bool(-3) is true, so boolAsInt * 2 = 2
    // BUF-NEXT: 2
    outputBuffer[5] = intAsBool ? uint(boolAsInt * 2) : 0;
}
