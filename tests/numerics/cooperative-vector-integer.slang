//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xdxc -Vd -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -experimental-feature

// CHECK: type: int32_t
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: -13
// CHECK-NEXT: 15
// CHECK-NEXT: 4
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 14
// CHECK-NEXT: 20

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
import slang.numerics;

RWStructuredBuffer<int> outputBuffer;

T integerArithmetic<T : IIntegerType>(T left, T right)
{
    return left / right + left % right;
}

T integerBitwise<T : IIntegerType>(T left, T right)
{
    return (left & right) | (left ^ ~right);
}

T integerShift<T : IIntegerType>(T value, T.ShiftAmount left, T.ShiftAmount right)
{
    value <<= left;
    return value >> right;
}

T signedIntegerAbsolute<T : ISignedIntegerType>(T value)
{
    return value.absoluteValue();
}

T unsignedIntegerIdentity<T : IUnsignedIntegerType>(T value)
{
    return value;
}

T.Mask integerLessThan<T : IIntegerType>(T left, T right)
{
    return left < right;
}

void storeMask<T : IIntegerType>(T left, T right, uint offset)
{
    T.Mask equalMask = left == right;
    T.Mask orderedMask = left <= right;
    outputBuffer[offset] = int(equalMask.any());
    outputBuffer[offset + 1] = int(orderedMask.all());
}

[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<int, 2> left = CoopVec<int, 2>(7, 10);
    CoopVec<int, 2> right = CoopVec<int, 2>(3, 4);
    CoopVec<uint, 2> unsignedValue = CoopVec<uint, 2>(15, 8);

    let arithmetic = integerArithmetic(left, right);
    let bitwise = integerBitwise(unsignedValue, CoopVec<uint, 2>(3, 1));
    let absoluteValue = signedIntegerAbsolute(CoopVec<int, 2>(-4, 5));
    let shifted = integerShift(left, CoopVec<int, 2>(1, 2), CoopVec<int, 2>(0, 1));
    let unsignedResult = unsignedIntegerIdentity(unsignedValue);

    outputBuffer[0] = arithmetic[0];
    outputBuffer[1] = arithmetic[1];
    outputBuffer[2] = int(bitwise[0]);
    outputBuffer[3] = int(unsignedResult[0]);
    outputBuffer[4] = absoluteValue[0];
    storeMask(left, CoopVec<int, 2>(7, 11), 5);
    outputBuffer[7] = int(integerLessThan(left, right)[0]);
    outputBuffer[8] = shifted[0];
    outputBuffer[9] = shifted[1];
}
