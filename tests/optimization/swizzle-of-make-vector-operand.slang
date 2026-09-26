// A swizzle of a vector constructor selects its lanes directly from the
// constructor's operands, including operands that are themselves vectors.
// For example, `uint3(a, 0).xy` is just `a`, so no wider vector is built.
// Related to #13263.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type

//TEST_INPUT:ubuffer(data=[1 2 3 4 5], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// CHECK-LABEL: %wholeOperand = OpFunction
// CHECK-NEXT: %[[A:[A-Za-z0-9_]+]] = OpFunctionParameter %v2uint
// CHECK-NEXT: OpLabel
// CHECK-NEXT: OpReturnValue %[[A]]
uint2 wholeOperand(uint2 a)
{
    return uint3(a, 0).xy;
}

// CHECK-LABEL: %reorderedOperand = OpFunction
// CHECK-NEXT: %[[A:[A-Za-z0-9_]+]] = OpFunctionParameter %v2uint
// CHECK-NEXT: OpFunctionParameter %uint
// CHECK-NEXT: OpLabel
// CHECK-NEXT: %[[R:[0-9]+]] = OpVectorShuffle %v2uint %[[A]] %[[A]] 1 0
// CHECK-NEXT: OpReturnValue %[[R]]
uint2 reorderedOperand(uint2 a, uint s)
{
    return uint3(a, s).yx;
}

// CHECK-LABEL: %crossOperand = OpFunction
// CHECK-NEXT: %[[A:[A-Za-z0-9_]+]] = OpFunctionParameter %v2uint
// CHECK-NEXT: %[[B:[A-Za-z0-9_]+]] = OpFunctionParameter %v2uint
// CHECK-NEXT: OpLabel
// CHECK-NEXT: %[[AY:[0-9]+]] = OpCompositeExtract %uint %[[A]] 1
// CHECK-NEXT: %[[BX:[0-9]+]] = OpCompositeExtract %uint %[[B]] 0
// CHECK-NEXT: %[[R:[0-9]+]] = OpCompositeConstruct %v2uint %[[AY]] %[[BX]]
// CHECK-NEXT: OpReturnValue %[[R]]
uint2 crossOperand(uint2 a, uint2 b)
{
    return uint4(a, b).yz;
}

// CHECK-LABEL: %trailingOperand = OpFunction
// CHECK-NEXT: OpFunctionParameter %uint
// CHECK-NEXT: %[[B:[A-Za-z0-9_]+]] = OpFunctionParameter %v2uint
// CHECK-NEXT: OpLabel
// CHECK-NEXT: OpReturnValue %[[B]]
uint2 trailingOperand(uint s, uint2 b)
{
    return uint3(s, b).yz;
}

// CHECK-LABEL: %mixedOperands = OpFunction
// CHECK-NEXT: %[[A:[A-Za-z0-9_]+]] = OpFunctionParameter %v2uint
// CHECK-NEXT: %[[S:[A-Za-z0-9_]+]] = OpFunctionParameter %uint
// CHECK-NEXT: OpLabel
// CHECK-NEXT: %[[AX:[0-9]+]] = OpCompositeExtract %uint %[[A]] 0
// CHECK-NEXT: %[[R:[0-9]+]] = OpCompositeConstruct %v2uint %[[S]] %[[AX]]
// CHECK-NEXT: OpReturnValue %[[R]]
uint2 mixedOperands(uint2 a, uint s)
{
    return uint3(a, s).zx;
}

// CHECK-LABEL: %singleLane = OpFunction
// CHECK-NEXT: %[[A:[A-Za-z0-9_]+]] = OpFunctionParameter %v2uint
// CHECK-NEXT: OpFunctionParameter %uint
// CHECK-NEXT: OpLabel
// CHECK-NEXT: %[[R:[0-9]+]] = OpCompositeExtract %uint %[[A]] 1
// CHECK-NEXT: OpReturnValue %[[R]]
uint singleLane(uint2 a, uint s)
{
    return uint3(a, s).y;
}

// CHECK-LABEL: %repeatedLanes = OpFunction
// CHECK-NEXT: %[[A:[A-Za-z0-9_]+]] = OpFunctionParameter %v2uint
// CHECK-NEXT: OpFunctionParameter %uint
// CHECK-NEXT: OpLabel
// CHECK-NEXT: %[[R:[0-9]+]] = OpVectorShuffle %v3uint %[[A]] %[[A]] 1 0 1
// CHECK-NEXT: OpReturnValue %[[R]]
uint3 repeatedLanes(uint2 a, uint s)
{
    return uint3(a, s).yxy;
}

// The swizzle first folds to a swizzle of the inner `uint3(a, s)`, which then
// folds again to a swizzle of `a`.
// CHECK-LABEL: %nestedOperand = OpFunction
// CHECK-NEXT: %[[A:[A-Za-z0-9_]+]] = OpFunctionParameter %v2uint
// CHECK-NEXT: OpFunctionParameter %uint
// CHECK-NEXT: OpFunctionParameter %uint
// CHECK-NEXT: OpLabel
// CHECK-NEXT: %[[R:[0-9]+]] = OpVectorShuffle %v2uint %[[A]] %[[A]] 1 0
// CHECK-NEXT: OpReturnValue %[[R]]
uint2 nestedOperand(uint2 a, uint s, uint t)
{
    return uint4(uint3(a, s), t).yx;
}

void store(uint index, uint2 value)
{
    outputBuffer[index] = value.x;
    outputBuffer[index + 1] = value.y;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    uint2 a = uint2(inputBuffer[0], inputBuffer[1]);
    uint2 b = uint2(inputBuffer[2], inputBuffer[3]);
    uint s = inputBuffer[4];

    // BUF: 1
    // BUF-NEXT: 2
    store(0, wholeOperand(a));
    // BUF-NEXT: 2
    // BUF-NEXT: 1
    store(2, reorderedOperand(a, s));
    // BUF-NEXT: 2
    // BUF-NEXT: 3
    store(4, crossOperand(a, b));
    // BUF-NEXT: 3
    // BUF-NEXT: 4
    store(6, trailingOperand(s, b));
    // BUF-NEXT: 5
    // BUF-NEXT: 1
    store(8, mixedOperands(a, s));
    // BUF-NEXT: 2
    outputBuffer[10] = singleLane(a, s);
    // BUF-NEXT: 2
    // BUF-NEXT: 1
    // BUF-NEXT: 2
    uint3 repeated = repeatedLanes(a, s);
    outputBuffer[11] = repeated.x;
    outputBuffer[12] = repeated.y;
    outputBuffer[13] = repeated.z;
    // BUF-NEXT: 2
    // BUF-NEXT: 1
    store(14, nestedOperand(a, s, b.x));
}
