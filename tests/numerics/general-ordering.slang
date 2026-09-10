// General equality, ordering, and real-ordering-function contracts do not require numeric
// arithmetic or a Scalar and Mask. Application types can use these operations without claiming
// unrelated numeric capabilities.

//TEST:SIMPLE: -target hlsl -entry main -stage compute -o numeric-interfaces-general-ordering.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry main -stage compute -o numeric-interfaces-general-ordering.spv -experimental-feature

import slang.numerics;

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

struct ExtremaOnly : IRealOrderingFunctions
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

    This step(This edge)
    {
        return {value < edge.value ? 0 : 1};
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

T boundedMedian<T : IRealOrderingFunctions>(T x, T y, T z)
{
    return clamp(median3(x, y, z), min(x, y), max(y, z));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    NotificationKind ordinary = {1};
    NotificationKind urgent = {3};
    outputBuffer[0] = int(canInterrupt(urgent, ordinary));
    outputBuffer[1] = int(representsSameKind(ordinary, ordinary));
    ExtremaOnly bounded = boundedMedian(ExtremaOnly(1), ExtremaOnly(3), ExtremaOnly(2));
    outputBuffer[2] = bounded.value;
}
