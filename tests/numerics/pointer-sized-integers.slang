// Pointer-sized integers participate in the numeric and total-order interfaces like the other
// builtin integer types. SPIR-V code generation for intptr_t currently has an unrelated
// SizeAndAlignment limitation, so keep this conformance check on HLSL.

//TEST:SIMPLE: -target hlsl -entry main -stage compute -o numeric-interfaces-pointer-sized.hlsl -experimental-feature

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;

T addValues<T : INumeric>(T left, T right)
{
    return left + right;
}

bool lessThanValues<T : ITotallyOrdered>(T left, T right)
{
    return left < right;
}

T integerOperations<T : IIntegerType>(T left, T right)
{
    return (left / right) + (left % right) + (left & right);
}

T signedIntegerValue<T : IScalarSignedIntegerType>(T value)
{
    return -value;
}

T unsignedIntegerValue<T : IScalarUnsignedIntegerType>(T value)
{
    return value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    intptr_t signedValue = addValues(intptr_t(1), intptr_t(2));
    uintptr_t unsignedValue = addValues(uintptr_t(1), uintptr_t(2));
    bool ordered = lessThanValues(signedValue, intptr_t(4));
    intptr_t signedIntegerResult = integerOperations(intptr_t(7), intptr_t(3));
    uintptr_t unsignedIntegerResult = integerOperations(uintptr_t(7), uintptr_t(3));
    outputBuffer[0] = int(signedValue) + int(unsignedValue) + int(ordered) +
        int(signedIntegerValue(signedIntegerResult)) +
        int(unsignedIntegerValue(unsignedIntegerResult));
}
