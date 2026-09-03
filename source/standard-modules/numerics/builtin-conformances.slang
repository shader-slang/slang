implementing numerics;

// These conformances are owned by `slang.numerics`, so importing this experimental module opts the
// builtin scalar types into its public numeric hierarchy without adding a dependency from `core`.
public extension bool : IBooleanMask
{
    [__unsafeForceInlineEarly] This logicalNot() { return !this; }
    [__unsafeForceInlineEarly] This logicalAnd(This other) { return this && other; }
    [__unsafeForceInlineEarly] This logicalOr(This other) { return this || other; }
    [__unsafeForceInlineEarly] bool any() { return __numericsBuiltinAny(this); }
    [__unsafeForceInlineEarly] bool all() { return __numericsBuiltinAll(this); }
}

#define NUMERICS_SCALAR_CONFORMANCE(TYPE)                                       \
    public extension TYPE                                                       \
        : IComponentwiseOrdered, IPartiallyOrdered, IScalarNumeric              \
    {                                                                            \
        typealias Scalar = TYPE;                                                 \
        typealias Mask = bool;                                                   \
                                                                                 \
        [__unsafeForceInlineEarly]                                               \
        static This fromScalar(Scalar value) { return value; }                   \
        [__unsafeForceInlineEarly]                                               \
        __init<TSource : __BuiltinIntegerType>(TSource value)                    \
        {                                                                        \
            this = __numericsBuiltinConstructFromInteger<TYPE>(value);           \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        static This zero() { return This(0); }                                   \
        [__unsafeForceInlineEarly]                                               \
        static This one() { return This(1); }                                    \
        [__unsafeForceInlineEarly]                                               \
        bool isEqualTo(This other)                                               \
        {                                                                        \
            return __numericsBuiltinEqual(this, other);                          \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        bool isLessThan(This other)                                              \
        {                                                                        \
            return __numericsBuiltinLess(this, other);                           \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        bool isLessThanOrEqualTo(This other)                                     \
        {                                                                        \
            return __numericsBuiltinLessEqual(this, other);                      \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This adding(This other) { return this + other; }                         \
        [__unsafeForceInlineEarly]                                               \
        This subtracting(This other) { return this - other; }                    \
        [__unsafeForceInlineEarly]                                               \
        This multipliedBy(This other) { return this * other; }                   \
    }

NUMERICS_SCALAR_CONFORMANCE(int8_t)
NUMERICS_SCALAR_CONFORMANCE(int16_t)
NUMERICS_SCALAR_CONFORMANCE(int)
NUMERICS_SCALAR_CONFORMANCE(int64_t)
NUMERICS_SCALAR_CONFORMANCE(intptr_t)
NUMERICS_SCALAR_CONFORMANCE(uint8_t)
NUMERICS_SCALAR_CONFORMANCE(uint16_t)
NUMERICS_SCALAR_CONFORMANCE(uint)
NUMERICS_SCALAR_CONFORMANCE(uint64_t)
NUMERICS_SCALAR_CONFORMANCE(uintptr_t)
NUMERICS_SCALAR_CONFORMANCE(half)
NUMERICS_SCALAR_CONFORMANCE(float)
NUMERICS_SCALAR_CONFORMANCE(double)

#undef NUMERICS_SCALAR_CONFORMANCE

#define NUMERICS_SIGNED_INTEGER_CONFORMANCE(TYPE)                               \
    public extension TYPE : IScalarSignedNumeric                                \
    {                                                                            \
        [__unsafeForceInlineEarly]                                               \
        This negated() { return -this; }                                         \
        [__unsafeForceInlineEarly]                                               \
        This absoluteValue() { return __numericInterfaceBuiltinAbs(this); }      \
    }

NUMERICS_SIGNED_INTEGER_CONFORMANCE(int8_t)
NUMERICS_SIGNED_INTEGER_CONFORMANCE(int16_t)
NUMERICS_SIGNED_INTEGER_CONFORMANCE(int)
NUMERICS_SIGNED_INTEGER_CONFORMANCE(int64_t)
NUMERICS_SIGNED_INTEGER_CONFORMANCE(intptr_t)

#undef NUMERICS_SIGNED_INTEGER_CONFORMANCE

#define NUMERICS_INTEGER_CONFORMANCE(TYPE)                                      \
    public extension TYPE : ITotallyOrdered                                     \
    {}                                                                           \
                                                                                 \
    public extension TYPE : IScalarIntegerType                                  \
    {                                                                            \
        typealias ShiftAmount = int;                                             \
                                                                                 \
        [__unsafeForceInlineEarly]                                               \
        This quotientWhenDividedBy(This divisor)                                 \
        {                                                                        \
            return __numericsBuiltinIntegerDivide(this, divisor);                \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This remainderWhenDividedBy(This divisor)                                \
        {                                                                        \
            return __numericsBuiltinIntegerRemainder(this, divisor);             \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This bitwiseAnd(This other)                                              \
        {                                                                        \
            return __numericsBuiltinBitAnd(this, other);                         \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This bitwiseOr(This other)                                               \
        {                                                                        \
            return __numericsBuiltinBitOr(this, other);                          \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This bitwiseXor(This other)                                              \
        {                                                                        \
            return __numericsBuiltinBitXor(this, other);                         \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This bitwiseNot() { return __numericsBuiltinBitNot(this); }              \
        [__unsafeForceInlineEarly]                                               \
        This shiftedLeftBy(int amount)                                           \
        {                                                                        \
            return __numericsBuiltinShiftLeft(this, amount);                     \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This shiftedRightBy(int amount)                                          \
        {                                                                        \
            return __numericsBuiltinShiftRight(this, amount);                    \
        }                                                                        \
    }

NUMERICS_INTEGER_CONFORMANCE(int8_t)
NUMERICS_INTEGER_CONFORMANCE(int16_t)
NUMERICS_INTEGER_CONFORMANCE(int)
NUMERICS_INTEGER_CONFORMANCE(int64_t)
NUMERICS_INTEGER_CONFORMANCE(intptr_t)
NUMERICS_INTEGER_CONFORMANCE(uint8_t)
NUMERICS_INTEGER_CONFORMANCE(uint16_t)
NUMERICS_INTEGER_CONFORMANCE(uint)
NUMERICS_INTEGER_CONFORMANCE(uint64_t)
NUMERICS_INTEGER_CONFORMANCE(uintptr_t)

#undef NUMERICS_INTEGER_CONFORMANCE

#define NUMERICS_SIGNED_INTEGER_REFINEMENT(TYPE)                                \
    public extension TYPE : IScalarSignedIntegerType                            \
    {}

NUMERICS_SIGNED_INTEGER_REFINEMENT(int8_t)
NUMERICS_SIGNED_INTEGER_REFINEMENT(int16_t)
NUMERICS_SIGNED_INTEGER_REFINEMENT(int)
NUMERICS_SIGNED_INTEGER_REFINEMENT(int64_t)
NUMERICS_SIGNED_INTEGER_REFINEMENT(intptr_t)

#undef NUMERICS_SIGNED_INTEGER_REFINEMENT

#define NUMERICS_UNSIGNED_INTEGER_REFINEMENT(TYPE)                              \
    public extension TYPE : IScalarUnsignedIntegerType                          \
    {}

NUMERICS_UNSIGNED_INTEGER_REFINEMENT(uint8_t)
NUMERICS_UNSIGNED_INTEGER_REFINEMENT(uint16_t)
NUMERICS_UNSIGNED_INTEGER_REFINEMENT(uint)
NUMERICS_UNSIGNED_INTEGER_REFINEMENT(uint64_t)
NUMERICS_UNSIGNED_INTEGER_REFINEMENT(uintptr_t)

#undef NUMERICS_UNSIGNED_INTEGER_REFINEMENT

#define NUMERICS_FRACTIONAL_SCALAR_CONFORMANCE(TYPE)                            \
    public extension TYPE : IScalarFractional                                   \
    {                                                                            \
        [__unsafeForceInlineEarly]                                               \
        __init<TSource : __BuiltinFloatingPointType>(TSource value)              \
        {                                                                        \
            this = __numericsBuiltinConstructFromFloatingPoint<TYPE>(value);     \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This adding(This other) { return this + other; }                         \
        [__unsafeForceInlineEarly]                                               \
        This subtracting(This other) { return this - other; }                    \
        [__unsafeForceInlineEarly]                                               \
        This multipliedBy(This other) { return this * other; }                   \
        [__unsafeForceInlineEarly]                                               \
        This dividedBy(This other) { return this / other; }                      \
        [__unsafeForceInlineEarly]                                               \
        This negated() { return -this; }                                         \
        [__unsafeForceInlineEarly]                                               \
        This absoluteValue() { return __numericInterfaceBuiltinAbs(this); }      \
        [__unsafeForceInlineEarly]                                               \
        This reciprocal() { return __numericInterfaceBuiltinRcp(this); }         \
    }

NUMERICS_FRACTIONAL_SCALAR_CONFORMANCE(half)
NUMERICS_FRACTIONAL_SCALAR_CONFORMANCE(float)
NUMERICS_FRACTIONAL_SCALAR_CONFORMANCE(double)

#undef NUMERICS_FRACTIONAL_SCALAR_CONFORMANCE

public extension<let N : int> vector<bool, N> : IBooleanMask
{
    [__unsafeForceInlineEarly]
    This logicalNot()
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = !this[index];
        return result;
    }

    [__unsafeForceInlineEarly]
    This logicalAnd(This other)
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = this[index] && other[index];
        return result;
    }

    [__unsafeForceInlineEarly]
    This logicalOr(This other)
    {
        This result;
        [ForceUnroll]
        for (int index = 0; index < N; ++index)
            result[index] = this[index] || other[index];
        return result;
    }

    [__unsafeForceInlineEarly] bool any() { return __numericsBuiltinAny(this); }
    [__unsafeForceInlineEarly] bool all() { return __numericsBuiltinAll(this); }
}

public extension<let R : int, let C : int, let L : int>
    matrix<bool, R, C, L> : IBooleanMask
{
    [__unsafeForceInlineEarly]
    This logicalNot()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = !this[row][column];
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This logicalAnd(This other)
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = this[row][column] && other[row][column];
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This logicalOr(This other)
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = this[row][column] || other[row][column];
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    bool any()
    {
        return __numericsBuiltinAny(this);
    }

    [__unsafeForceInlineEarly]
    bool all()
    {
        return __numericsBuiltinAll(this);
    }
}

public extension<T : __BuiltinIntegerType & IScalarIntegerType, let N : int>
    vector<T, N> : IIntegerType
{
    typealias Scalar = T;
    typealias Mask = vector<bool, N>;
    typealias ShiftAmount = vector<int, N>;

    [__unsafeForceInlineEarly] static This fromScalar(Scalar value) { return This(value); }
    [__unsafeForceInlineEarly]
    __init<TSource : __BuiltinIntegerType>(TSource value)
    {
        this = This.fromScalar(__numericsBuiltinConstructFromInteger<T>(value));
    }
    [__unsafeForceInlineEarly] static This zero() { return This(Scalar.zero()); }
    [__unsafeForceInlineEarly] static This one() { return This(Scalar.one()); }
    [__unsafeForceInlineEarly]
    Mask isEqualTo(This other) { return __numericsBuiltinEqual(this, other); }
    [__unsafeForceInlineEarly]
    Mask isLessThan(This other) { return __numericsBuiltinLess(this, other); }
    [__unsafeForceInlineEarly]
    Mask isLessThanOrEqualTo(This other)
    {
        return __numericsBuiltinLessEqual(this, other);
    }
    [__unsafeForceInlineEarly]
    This adding(This other) { return __numericsBuiltinAdd(this, other); }
    [__unsafeForceInlineEarly]
    This subtracting(This other) { return __numericsBuiltinSubtract(this, other); }
    [__unsafeForceInlineEarly]
    This multipliedBy(This other) { return __numericsBuiltinMultiply(this, other); }
    [__unsafeForceInlineEarly]
    This quotientWhenDividedBy(This divisor)
    {
        return __numericsBuiltinIntegerDivide(this, divisor);
    }
    [__unsafeForceInlineEarly]
    This remainderWhenDividedBy(This divisor)
    {
        return __numericsBuiltinIntegerRemainder(this, divisor);
    }
    [__unsafeForceInlineEarly]
    This bitwiseAnd(This other) { return __numericsBuiltinBitAnd(this, other); }
    [__unsafeForceInlineEarly]
    This bitwiseOr(This other) { return __numericsBuiltinBitOr(this, other); }
    [__unsafeForceInlineEarly]
    This bitwiseXor(This other) { return __numericsBuiltinBitXor(this, other); }
    [__unsafeForceInlineEarly]
    This bitwiseNot() { return __numericsBuiltinBitNot(this); }
    [__unsafeForceInlineEarly]
    This shiftedLeftBy(ShiftAmount amount)
    {
        return __numericsBuiltinShiftLeft(this, amount);
    }
    [__unsafeForceInlineEarly]
    This shiftedRightBy(ShiftAmount amount)
    {
        return __numericsBuiltinShiftRight(this, amount);
    }
}

public extension<
    T : __BuiltinIntegerType & __BuiltinSignedArithmeticType & IScalarSignedIntegerType,
    let N : int>
    vector<T, N> : ISignedIntegerType
{
    [__unsafeForceInlineEarly]
    This negated() { return __numericsBuiltinNegate(this); }
    [__unsafeForceInlineEarly]
    This absoluteValue() { return __numericInterfaceBuiltinAbs(this); }
}

public extension<T : __BuiltinIntegerType & IScalarUnsignedIntegerType, let N : int>
    vector<T, N> : IUnsignedIntegerType
{}

public extension<T : __BuiltinFloatingPointType & IScalarFractional, let N : int>
    vector<T, N> : IComponentwiseOrdered
{
    typealias Scalar = T;
    typealias Mask = vector<bool, N>;

    [__unsafeForceInlineEarly] static This fromScalar(Scalar value) { return This(value); }
    [__unsafeForceInlineEarly]
    Mask isEqualTo(This other) { return __numericsBuiltinEqual(this, other); }
    [__unsafeForceInlineEarly]
    Mask isLessThan(This other) { return __numericsBuiltinLess(this, other); }
    [__unsafeForceInlineEarly]
    Mask isLessThanOrEqualTo(This other)
    {
        return __numericsBuiltinLessEqual(this, other);
    }
}

public extension<T : __BuiltinFloatingPointType & IScalarFractional, let N : int>
    vector<T, N> : IFractional
{
    [__unsafeForceInlineEarly]
    __init<TSource : __BuiltinIntegerType>(TSource value)
    {
        this = This.fromScalar(__numericsBuiltinConstructFromInteger<T>(value));
    }
    [__unsafeForceInlineEarly]
    __init<TSource : __BuiltinFloatingPointType>(TSource value)
    {
        this = This.fromScalar(__numericsBuiltinConstructFromFloatingPoint<T>(value));
    }
    [__unsafeForceInlineEarly] static This zero() { return This(T.zero()); }
    [__unsafeForceInlineEarly] static This one() { return This(T.one()); }
    [__unsafeForceInlineEarly]
    This adding(This other) { return __numericsBuiltinAdd(this, other); }
    [__unsafeForceInlineEarly]
    This subtracting(This other) { return __numericsBuiltinSubtract(this, other); }
    [__unsafeForceInlineEarly]
    This multipliedBy(This other) { return __numericsBuiltinMultiply(this, other); }
    [__unsafeForceInlineEarly]
    This dividedBy(This other) { return __numericsBuiltinDivide(this, other); }
    [__unsafeForceInlineEarly]
    This negated() { return __numericsBuiltinNegate(this); }
    [__unsafeForceInlineEarly]
    This absoluteValue() { return __numericInterfaceBuiltinAbs(this); }
    [__unsafeForceInlineEarly]
    This reciprocal() { return __numericInterfaceBuiltinRcp(this); }
}

public extension<
    T : __BuiltinFloatingPointType & IScalarFractional,
    let R : int,
    let C : int,
    let L : int>
    matrix<T, R, C, L> : IComponentwiseOrdered
{
    typealias Scalar = T;
    typealias Mask = matrix<bool, R, C, L>;

    [__unsafeForceInlineEarly] static This fromScalar(Scalar value) { return This(value); }

    [__unsafeForceInlineEarly]
    Mask isEqualTo(This other)
    {
        Mask result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
            {
                result[row][column] =
                    __numericsBuiltinEqual(this[row][column], other[row][column]);
            }
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    Mask isLessThan(This other)
    {
        Mask result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
            {
                result[row][column] =
                    __numericsBuiltinLess(this[row][column], other[row][column]);
            }
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    Mask isLessThanOrEqualTo(This other)
    {
        Mask result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
            {
                result[row][column] =
                    __numericsBuiltinLessEqual(this[row][column], other[row][column]);
            }
        }
        return result;
    }
}

public extension<
    T : __BuiltinFloatingPointType & IScalarFractional,
    let R : int,
    let C : int,
    let L : int>
    matrix<T, R, C, L> : IFractional
{
    [__unsafeForceInlineEarly]
    __init<TSource : __BuiltinIntegerType>(TSource value)
    {
        this = This.fromScalar(__numericsBuiltinConstructFromInteger<T>(value));
    }
    [__unsafeForceInlineEarly]
    __init<TSource : __BuiltinFloatingPointType>(TSource value)
    {
        this = This.fromScalar(__numericsBuiltinConstructFromFloatingPoint<T>(value));
    }
    [__unsafeForceInlineEarly] static This zero() { return This(T.zero()); }
    [__unsafeForceInlineEarly] static This one() { return This(T.one()); }

    [__unsafeForceInlineEarly]
    This adding(This other) { return __numericsBuiltinAdd(this, other); }
    [__unsafeForceInlineEarly]
    This subtracting(This other) { return __numericsBuiltinSubtract(this, other); }
    [__unsafeForceInlineEarly]
    This multipliedBy(This other) { return __numericsBuiltinMultiply(this, other); }
    [__unsafeForceInlineEarly]
    This dividedBy(This other) { return __numericsBuiltinDivide(this, other); }
    [__unsafeForceInlineEarly]
    This negated() { return __numericsBuiltinNegate(this); }

    [__unsafeForceInlineEarly]
    This absoluteValue()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinAbs(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This reciprocal()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinRcp(this[row][column]);
        }
        return result;
    }
}
