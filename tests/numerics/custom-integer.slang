// Verify that user-defined scalar and shaped integer types can conform to the public integer
// interfaces and use the corresponding generic operators.

//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o numeric-interfaces-custom-integer.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o numeric-interfaces-custom-integer.spv -experimental-feature
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature

// CHECK: type: int32_t
// CHECK-NEXT: 44

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;

struct WrappedInt : IScalarSignedIntegerType
{
    int value;

    typealias Scalar = WrappedInt;
    typealias Mask = bool;
    typealias ShiftAmount = int;

    __init<TSource : __BuiltinIntegerType>(TSource value)
    {
        this.value = __intCast<int>(value);
    }

    static This fromScalar(Scalar value)
    {
        return value;
    }

    static This zero()
    {
        return {0};
    }

    static This one()
    {
        return {1};
    }

    bool isEqualTo(This other)
    {
        return value == other.value;
    }

    bool isLessThan(This other)
    {
        return value < other.value;
    }

    bool isLessThanOrEqualTo(This other)
    {
        return value <= other.value;
    }

    This plus(This other)
    {
        return {value + other.value};
    }

    This minus(This other)
    {
        return {value - other.value};
    }

    This multipliedBy(This other)
    {
        return {value * other.value};
    }

    This negated()
    {
        return {-value};
    }

    This absoluteValue()
    {
        return {abs(value)};
    }

    This quotientWhenDividedBy(This divisor)
    {
        return {value / divisor.value};
    }

    This remainderWhenDividedBy(This divisor)
    {
        return {value % divisor.value};
    }

    This bitwiseAnd(This other)
    {
        return {value & other.value};
    }

    This bitwiseOr(This other)
    {
        return {value | other.value};
    }

    This bitwiseXor(This other)
    {
        return {value ^ other.value};
    }

    This bitwiseNot()
    {
        return {~value};
    }

    This shiftedLeftBy(ShiftAmount amount)
    {
        return {value << amount};
    }

    This shiftedRightBy(ShiftAmount amount)
    {
        return {value >> amount};
    }
}

struct SignedIntegerPair<T : IScalarSignedIntegerType> : ISignedIntegerType
{
    T x;
    T y;

    typealias Scalar = T;
    typealias Mask = vector<bool, 2>;
    typealias ShiftAmount = vector<int, 2>;

    __init<TSource : __BuiltinIntegerType>(TSource value)
    {
        let scalar = T(value);
        x = scalar;
        y = scalar;
    }

    __init(T x, T y)
    {
        this.x = x;
        this.y = y;
    }

    static This fromScalar(Scalar value)
    {
        return {value, value};
    }

    static This zero()
    {
        return fromScalar(Scalar.zero());
    }

    static This one()
    {
        return fromScalar(Scalar.one());
    }

    Mask isEqualTo(This other)
    {
        return Mask(x == other.x, y == other.y);
    }

    Mask isLessThan(This other)
    {
        return Mask(x < other.x, y < other.y);
    }

    Mask isLessThanOrEqualTo(This other)
    {
        return Mask(x <= other.x, y <= other.y);
    }

    This plus(This other)
    {
        return {x + other.x, y + other.y};
    }

    This minus(This other)
    {
        return {x - other.x, y - other.y};
    }

    This multipliedBy(This other)
    {
        return {x * other.x, y * other.y};
    }

    This negated()
    {
        return {-x, -y};
    }

    This absoluteValue()
    {
        return {abs(x), abs(y)};
    }

    This quotientWhenDividedBy(This divisor)
    {
        return {x / divisor.x, y / divisor.y};
    }

    This remainderWhenDividedBy(This divisor)
    {
        return {x % divisor.x, y % divisor.y};
    }

    This bitwiseAnd(This other)
    {
        return {x & other.x, y & other.y};
    }

    This bitwiseOr(This other)
    {
        return {x | other.x, y | other.y};
    }

    This bitwiseXor(This other)
    {
        return {x ^ other.x, y ^ other.y};
    }

    This bitwiseNot()
    {
        return {~x, ~y};
    }

    This shiftedLeftBy(ShiftAmount amount)
    {
        return {
            x.shiftedLeftBy(T.ShiftAmount(amount.x)),
            y.shiftedLeftBy(T.ShiftAmount(amount.y)),
        };
    }

    This shiftedRightBy(ShiftAmount amount)
    {
        return {
            x.shiftedRightBy(T.ShiftAmount(amount.x)),
            y.shiftedRightBy(T.ShiftAmount(amount.y)),
        };
    }
}

T integerCurve<T : IIntegerType>(T left, T right)
{
    T one = T.one();
    return (left / right + left % right) ^ ((left & right) | ~one);
}

T integerCompoundCurve<T : IIntegerType>(T left, T right)
{
    left += right;
    left -= right;
    left *= right;
    left /= right;
    left %= right;
    left &= right;
    left |= right;
    left ^= right;
    return left;
}

T integerIncrementCurve<T : IIntegerType>(T value)
{
    T result = +value;
    result += ++value;
    result += value++;
    result += --value;
    result += value--;
    return result;
}

T integerShiftCurve<T : IIntegerType>(T value, T.ShiftAmount left, T.ShiftAmount right)
{
    value <<= left;
    return value >> right;
}

T constructFromBuiltinInteger<T : INumeric, TSource : __BuiltinIntegerType>(TSource value)
{
    return T(value);
}

T.Mask orderedMask<T : IIntegerType>(T left, T right)
{
    return left <= right;
}

T absoluteInteger<T : ISignedIntegerType>(T value)
{
    return abs(value);
}

T scalarIdentity<T : IScalarSignedIntegerType>(T value)
{
    return value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    WrappedInt scalar = scalarIdentity(integerCurve(
        constructFromBuiltinInteger<WrappedInt>(int8_t(7)), WrappedInt(3)));

    SignedIntegerPair<WrappedInt> left = {WrappedInt(-7), WrappedInt(10)};
    SignedIntegerPair<WrappedInt> right = {WrappedInt(3), WrappedInt(4)};
    let shaped = integerCurve(left, right);
    let compound = integerCompoundCurve(left, right);
    let incremented = integerIncrementCurve(left);
    let shifted = integerShiftCurve(left, int2(1, 2), int2(0, 1));
    let magnitude = absoluteInteger(left);
    let mask = orderedMask(left, right);
    let wideConstructed = constructFromBuiltinInteger<SignedIntegerPair<WrappedInt>>(uint64_t(5));

    outputBuffer[0] = scalar.value + shaped.x.value + shaped.y.value + compound.x.value +
        compound.y.value + incremented.x.value + incremented.y.value + magnitude.x.value +
        magnitude.y.value + shifted.x.value + shifted.y.value + int(mask.any()) +
        int(mask.all()) + wideConstructed.x.value;
}
