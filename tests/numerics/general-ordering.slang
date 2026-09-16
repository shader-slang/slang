//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o numeric-interfaces-general-ordering.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o numeric-interfaces-general-ordering.spv -experimental-feature

// General equality, ordering, numerical-extrema, and real-ordering-function contracts do not
// require numeric arithmetic or a Scalar and Mask. Application types can use these operations
// without claiming unrelated numeric capabilities.

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 5
// CHECK-NEXT: 0
// CHECK-NEXT: 4
// CHECK-NEXT: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 2
// CHECK-NEXT: 2
// CHECK-NEXT: -2
// CHECK-NEXT: 7
// CHECK-NEXT: 0
// CHECK-NEXT: 4
// CHECK-NEXT: 7
// CHECK-NEXT: 2

import slang.numerics;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct NotificationKind : IPartiallyOrdered
{
    int interruptionPriority;

    bool isEqualTo(This other)
    {
        return interruptionPriority == other.interruptionPriority;
    }

    bool isLessThan(This other)
    {
        return interruptionPriority < other.interruptionPriority;
    }

    bool isLessThanOrEqualTo(This other)
    {
        return interruptionPriority <= other.interruptionPriority;
    }
}

struct ExtremaOnly : INumericExtrema
{
    int value;

    This minimum(This other)
    {
        return {value < other.value ? value : other.value};
    }

    This maximum(This other)
    {
        return {value > other.value ? value : other.value};
    }
}

bool canInterrupt<T : IPartiallyOrdered>(T incoming, T active)
{
    return incoming >= active;
}

bool representsSameKind<T : IEquatable>(T left, T right)
{
    return left == right;
}

T boundedMedian<T : INumericExtrema>(T x, T y, T z)
{
    return clamp(median3(x, y, z), min(x, y), max(y, z));
}

T bound<T : INumericExtrema>(T value, T lower, T upper)
{
    return clamp(value, lower, upper);
}

T threeWayMinimum<T : INumericExtrema>(T x, T y, T z)
{
    return min3(x, y, z);
}

T threeWayMaximum<T : INumericExtrema>(T x, T y, T z)
{
    return max3(x, y, z);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    NotificationKind ordinary = {1};
    NotificationKind urgent = {3};
    outputBuffer[0] = int(canInterrupt(urgent, ordinary));
    outputBuffer[1] = int(representsSameKind(ordinary, ordinary));
    ExtremaOnly bounded = boundedMedian(ExtremaOnly(1), ExtremaOnly(3), ExtremaOnly(2));
    outputBuffer[2] = bounded.value;
    outputBuffer[3] = bound(8, 0, 5);
    int3 boundedInteger = bound(int3(-2, 4, 9), int3(0, 1, 2), int3(3, 5, 7));
    outputBuffer[4] = boundedInteger.x;
    outputBuffer[5] = boundedInteger.y;
    outputBuffer[6] = boundedInteger.z;

    uint3 boundedUnsigned = bound(uint3(10, 1, 0), uint3(2, 2, 2), uint3(8, 8, 8));
    outputBuffer[7] = int(boundedUnsigned.x);
    outputBuffer[8] = int(boundedUnsigned.y);
    outputBuffer[9] = int(boundedUnsigned.z);
    outputBuffer[10] = threeWayMinimum(7, -2, 4);
    outputBuffer[11] = threeWayMaximum(7, -2, 4);

    int2x2 boundedIntegerMatrix = bound(
        int2x2(-2, 4, 9, 1),
        int2x2(0, 1, 2, 2),
        int2x2(3, 5, 7, 6));
    outputBuffer[12] = boundedIntegerMatrix[0][0];
    outputBuffer[13] = boundedIntegerMatrix[0][1];
    outputBuffer[14] = boundedIntegerMatrix[1][0];
    outputBuffer[15] = boundedIntegerMatrix[1][1];
}
