// Verify that user-defined scalar and shaped integer types can conform to the public integer
// interfaces and use the corresponding generic operators.

//TEST:SIMPLE: -target hlsl -entry main -stage compute -o numeric-interfaces-custom-integer.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry main -stage compute -o numeric-interfaces-custom-integer.spv -experimental-feature

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;

struct WrappedInt : IScalarSignedIntegerType
{
    int value;

    typealias Scalar = WrappedInt;
    typealias Mask = bool;
    typealias ShiftAmount = int;

    __init(int value)
    {
        this.value = value;
    }

    static This fromScalar(Scalar value)
    {
        return value;
    }

    static Scalar getZero()
    {
        return {0};
    }

    static Scalar getOne()
    {
        return {1};
    }

    bool isEqual(This other)
    {
        return value == other.value;
    }

    bool isLessThan(This other)
    {
        return value < other.value;
    }

    bool isLessThanOrEqual(This other)
    {
        return value <= other.value;
    }

    bool equals(This other)
    {
        return isEqual(other);
    }

    bool lessThan(This other)
    {
        return isLessThan(other);
    }

    bool lessThanOrEquals(This other)
    {
        return isLessThanOrEqual(other);
    }

    This add(This other)
    {
        return {value + other.value};
    }

    This sub(This other)
    {
        return {value - other.value};
    }

    This mul(This other)
    {
        return {value * other.value};
    }

    This neg()
    {
        return {-value};
    }

    This absoluteValue()
    {
        return {abs(value)};
    }

    This quotient(This divisor)
    {
        return {value / divisor.value};
    }

    This remainder(This divisor)
    {
        return {value % divisor.value};
    }

    This bitAnd(This other)
    {
        return {value & other.value};
    }

    This bitOr(This other)
    {
        return {value | other.value};
    }

    This bitXor(This other)
    {
        return {value ^ other.value};
    }

    This bitNot()
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

    __init(int value)
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

    static Scalar getZero()
    {
        return Scalar.getZero();
    }

    static Scalar getOne()
    {
        return Scalar.getOne();
    }

    Mask isEqual(This other)
    {
        return Mask(x == other.x, y == other.y);
    }

    Mask isLessThan(This other)
    {
        return Mask(x < other.x, y < other.y);
    }

    Mask isLessThanOrEqual(This other)
    {
        return Mask(x <= other.x, y <= other.y);
    }

    This add(This other)
    {
        return {x + other.x, y + other.y};
    }

    This sub(This other)
    {
        return {x - other.x, y - other.y};
    }

    This mul(This other)
    {
        return {x * other.x, y * other.y};
    }

    This neg()
    {
        return {-x, -y};
    }

    This absoluteValue()
    {
        return {abs(x), abs(y)};
    }

    This quotient(This divisor)
    {
        return {x / divisor.x, y / divisor.y};
    }

    This remainder(This divisor)
    {
        return {x % divisor.x, y % divisor.y};
    }

    This bitAnd(This other)
    {
        return {x & other.x, y & other.y};
    }

    This bitOr(This other)
    {
        return {x | other.x, y | other.y};
    }

    This bitXor(This other)
    {
        return {x ^ other.x, y ^ other.y};
    }

    This bitNot()
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
    T one = T.fromScalar(T.getOne());
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

T constructFromIntegerLiteralType<T : INumeric>(int value)
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
void main()
{
    WrappedInt scalar = scalarIdentity(integerCurve(
        constructFromIntegerLiteralType<WrappedInt>(7), WrappedInt(3)));

    SignedIntegerPair<WrappedInt> left = {WrappedInt(-7), WrappedInt(10)};
    SignedIntegerPair<WrappedInt> right = {WrappedInt(3), WrappedInt(4)};
    let shaped = integerCurve(left, right);
    let compound = integerCompoundCurve(left, right);
    let incremented = integerIncrementCurve(left);
    let shifted = integerShiftCurve(left, int2(1, 2), int2(0, 1));
    let magnitude = absoluteInteger(left);
    let mask = orderedMask(left, right);

    outputBuffer[0] = scalar.value + shaped.x.value + shaped.y.value + compound.x.value +
        compound.y.value + incremented.x.value + incremented.y.value + magnitude.x.value +
        magnitude.y.value + shifted.x.value + shifted.y.value + int(mask.any()) +
        int(mask.all());
}
