//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Test that bool <-> integer specialization constant casts produce valid SPIR-V.
//
// Bool-to-integer uses OpSelect (UConvert/SConvert cannot accept bool operands).
// Integer-to-bool uses INotEqual (UConvert/SConvert cannot produce bool results).

// CHECK-DAG: %[[BTRUE:[0-9A-Za-z_]+]] = OpSpecConstantTrue %bool
// CHECK-DAG: %[[BFALSE:[0-9A-Za-z_]+]] = OpSpecConstantFalse %bool
// CHECK-DAG: %[[UVAL:[0-9A-Za-z_]+]] = OpSpecConstant %uint 5
// CHECK-DAG: %[[UZVAL:[0-9A-Za-z_]+]] = OpSpecConstant %uint 0
// CHECK-DAG: %[[IVAL:[0-9A-Za-z_]+]] = OpSpecConstant %int -3

// bool -> uint: OpSelect picking 1 or 0
// CHECK-DAG: %[[UONE:[0-9A-Za-z_]+]] = OpConstant %uint 1
// CHECK-DAG: %[[UZERO:[0-9A-Za-z_]+]] = OpConstant %uint 0
// CHECK-DAG: OpSpecConstantOp %uint Select %[[BTRUE]] %[[UONE]] %[[UZERO]]
// CHECK-DAG: OpSpecConstantOp %uint Select %[[BFALSE]] %[[UONE]] %[[UZERO]]

// bool -> int: OpSelect picking 1 or 0
// CHECK-DAG: %[[IONE:[0-9A-Za-z_]+]] = OpConstant %int 1
// CHECK-DAG: %[[IZERO:[0-9A-Za-z_]+]] = OpConstant %int 0
// CHECK-DAG: OpSpecConstantOp %int Select %[[BTRUE]] %[[IONE]] %[[IZERO]]

// uint -> bool: INotEqual against 0
// CHECK-DAG: OpSpecConstantOp %bool INotEqual %[[UVAL]] %[[UZERO]]
// CHECK-DAG: OpSpecConstantOp %bool INotEqual %[[UZVAL]] %[[UZERO]]

// int -> bool: INotEqual against 0
// CHECK-DAG: OpSpecConstantOp %bool INotEqual %[[IVAL]] %[[IZERO]]

[SpecializationConstant]
const bool BTrue = true;

[SpecializationConstant]
const bool BFalse = false;

[SpecializationConstant]
const uint U = 5;

[SpecializationConstant]
const uint UZero = 0;

[SpecializationConstant]
const int I = -3;

static const uint trueAsUint = uint(BTrue);
static const uint falseAsUint = uint(BFalse);
static const int trueAsInt = int(BTrue);
static const bool uintAsBool = bool(U);
static const bool uintZeroAsBool = bool(UZero);
static const bool intAsBool = bool(I);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // uint(true) = 1
    // BUF: 1
    outputBuffer[0] = trueAsUint;

    // uint(false) = 0
    // BUF-NEXT: 0
    outputBuffer[1] = falseAsUint;

    // int(true) = 1
    // BUF-NEXT: 1
    outputBuffer[2] = uint(trueAsInt);

    // uint(bool(5)) = uint(true) = 1
    // BUF-NEXT: 1
    outputBuffer[3] = uint(uintAsBool);

    // uint(bool(0)) = uint(false) = 0
    // BUF-NEXT: 0
    outputBuffer[4] = uint(uintZeroAsBool);

    // int(-3) != 0, so bool(-3) = true, uint(true) = 1
    // BUF-NEXT: 1
    outputBuffer[5] = uint(intAsBool);

    // bool(5) is true, so trueAsUint + U = 1 + 5 = 6
    // BUF-NEXT: 6
    outputBuffer[6] = uintAsBool ? trueAsUint + U : 0;

    // bool(-3) is true, so trueAsInt * 2 = 2
    // BUF-NEXT: 2
    outputBuffer[7] = intAsBool ? uint(trueAsInt * 2) : 0;

    // bool(0) is false, so ternary takes the else branch = 99
    // BUF-NEXT: 99
    outputBuffer[8] = uintZeroAsBool ? trueAsUint : 99;

    // false -> 0, so falseAsUint + 42 = 0 + 42 = 42
    // BUF-NEXT: 42
    outputBuffer[9] = falseAsUint + 42;
}
