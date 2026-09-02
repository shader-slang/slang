implementing numerics;

// The sealed builtin interfaces cannot directly refine the public scalar hierarchy today. The
// recursive `Scalar == This` constraints cause the conformance checker to construct an invalid
// witness while checking the builtin interface itself, so the concrete scalar types conform here.
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
        static Scalar getZero() { return Scalar(0); }                            \
        [__unsafeForceInlineEarly]                                               \
        static Scalar getOne() { return Scalar(1); }                             \
        [__unsafeForceInlineEarly]                                               \
        bool isEqual(This other) { return __numericsBuiltinEqual(this, other); } \
        [__unsafeForceInlineEarly]                                               \
        bool isLessThan(This other)                                              \
        {                                                                        \
            return __numericsBuiltinLess(this, other);                           \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        bool isLessThanOrEqual(This other)                                       \
        {                                                                        \
            return __numericsBuiltinLessEqual(this, other);                      \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        bool equals(This other) { return __numericsBuiltinEqual(this, other); }  \
        [__unsafeForceInlineEarly]                                               \
        bool lessThan(This other) { return __numericsBuiltinLess(this, other); } \
        [__unsafeForceInlineEarly]                                               \
        bool lessThanOrEquals(This other)                                        \
        {                                                                        \
            return __numericsBuiltinLessEqual(this, other);                      \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This add(This other) { return __numericsLegacyAdd(this, other); }        \
        [__unsafeForceInlineEarly]                                               \
        This sub(This other) { return __numericsLegacySub(this, other); }        \
        [__unsafeForceInlineEarly]                                               \
        This mul(This other) { return __numericsLegacyMul(this, other); }        \
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
        This neg() { return __numericsLegacyNeg(this); }                         \
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
        This quotient(This divisor)                                              \
        {                                                                        \
            return __numericsBuiltinIntegerDivide(this, divisor);                \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This remainder(This divisor)                                             \
        {                                                                        \
            return __numericsBuiltinIntegerRemainder(this, divisor);             \
        }                                                                        \
        [__unsafeForceInlineEarly]                                               \
        This bitAnd(This other) { return __numericsBuiltinBitAnd(this, other); } \
        [__unsafeForceInlineEarly]                                               \
        This bitOr(This other) { return __numericsBuiltinBitOr(this, other); }   \
        [__unsafeForceInlineEarly]                                               \
        This bitXor(This other) { return __numericsBuiltinBitXor(this, other); } \
        [__unsafeForceInlineEarly]                                               \
        This bitNot() { return __numericsBuiltinBitNot(this); }                  \
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
        This add(This other) { return __numericsLegacyAdd(this, other); }        \
        [__unsafeForceInlineEarly]                                               \
        This sub(This other) { return __numericsLegacySub(this, other); }        \
        [__unsafeForceInlineEarly]                                               \
        This mul(This other) { return __numericsLegacyMul(this, other); }        \
        [__unsafeForceInlineEarly]                                               \
        This div(This other) { return __numericsLegacyDiv(this, other); }        \
        [__unsafeForceInlineEarly]                                               \
        This neg() { return __numericsLegacyNeg(this); }                         \
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
    [__unsafeForceInlineEarly] static Scalar getZero() { return Scalar.getZero(); }
    [__unsafeForceInlineEarly] static Scalar getOne() { return Scalar.getOne(); }
    [__unsafeForceInlineEarly]
    Mask isEqual(This other) { return __numericsBuiltinEqual(this, other); }
    [__unsafeForceInlineEarly]
    Mask isLessThan(This other) { return __numericsBuiltinLess(this, other); }
    [__unsafeForceInlineEarly]
    Mask isLessThanOrEqual(This other)
    {
        return __numericsBuiltinLessEqual(this, other);
    }
    [__unsafeForceInlineEarly]
    This quotient(This divisor) { return __numericsBuiltinIntegerDivide(this, divisor); }
    [__unsafeForceInlineEarly]
    This remainder(This divisor) { return __numericsBuiltinIntegerRemainder(this, divisor); }
    [__unsafeForceInlineEarly]
    This bitAnd(This other) { return __numericsBuiltinBitAnd(this, other); }
    [__unsafeForceInlineEarly]
    This bitOr(This other) { return __numericsBuiltinBitOr(this, other); }
    [__unsafeForceInlineEarly]
    This bitXor(This other) { return __numericsBuiltinBitXor(this, other); }
    [__unsafeForceInlineEarly]
    This bitNot() { return __numericsBuiltinBitNot(this); }
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
    Mask isEqual(This other) { return __numericsBuiltinEqual(this, other); }
    [__unsafeForceInlineEarly]
    Mask isLessThan(This other) { return __numericsBuiltinLess(this, other); }
    [__unsafeForceInlineEarly]
    Mask isLessThanOrEqual(This other)
    {
        return __numericsBuiltinLessEqual(this, other);
    }
}

public extension<T : __BuiltinFloatingPointType & IScalarFractional, let N : int>
    vector<T, N> : IFractional
{
    [__unsafeForceInlineEarly] static T getZero() { return T.getZero(); }
    [__unsafeForceInlineEarly] static T getOne() { return T.getOne(); }
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
    Mask isEqual(This other)
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
    Mask isLessThanOrEqual(This other)
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
    [__unsafeForceInlineEarly] static T getZero() { return T.getZero(); }
    [__unsafeForceInlineEarly] static T getOne() { return T.getOne(); }

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
