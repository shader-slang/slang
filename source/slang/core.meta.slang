// Slang `core` library

// Aliases for base types
typedef half float16_t;
typedef float float32_t;
typedef double float64_t;

typedef int int32_t;
typedef uint uint32_t;

typedef uintptr_t size_t;
typedef uintptr_t usize_t
typedef intptr_t ssize_t;

// Modifier for variables that must resolve to compile-time constants
// as part of translation.
syntax constexpr : ConstExprModifier;

// Modifier for variables that should have writes be made
// visible at the global-memory scope
syntax globallycoherent : GloballyCoherentModifier;

/// Modifier to disable inteprolation and force per-vertex passing of a varying attribute.
///
/// When a varying attribute passed to the fragment shader is marked `pervertex`, it will
/// not be interpolated during rasterization (similar to `nointerpolate` attributes).
/// Unlike a plain `nointerpolate` attribute, this modifier indicates that the attribute
/// should *only* be acccessed through the `GetAttributeAtVertex()` operation, so access its
/// distinct per-vertex values.
///
syntax pervertex : PerVertexModifier;

/// Modifier to indicate a buffer or texture element type is
/// backed by data in an unsigned normalized format.
///
/// The `unorm` modifier is only valid on `float` and `vector`s
/// with `float` elements.
///
/// This modifier does not affect the semantics of any variable,
/// parameter, or field that uses it. The semantics of a `float`
/// or vector are the same with or without `unorm`.
///
/// The `unorm` modifier can be used for the element type of a
/// buffer or texture, to indicate that the data that is bound
/// to that buffer or texture is in a matching normalized format.
/// Some platforms may require a `unorm` qualifier for such buffers
/// and textures, and others may operate correctly without it.
///
syntax unorm : UNormModifier;

/// Modifier to indicate a buffer or texture element type is
/// backed by data in an signed normalized format.
///
/// The `snorm` modifier is only valid on `float` and `vector`s
/// with `float` elements.
///
/// This modifier does not affect the semantics of any variable,
/// parameter, or field that uses it. The semantics of a `float`
/// or vector are the same with or without `snorm`.
///
/// The `snorm` modifier can be used for the element type of a
/// buffer or texture, to indicate that the data that is bound
/// to that buffer or texture is in a matching normalized format.
/// Some platforms may require a `unorm` qualifier for such buffers
/// and textures, and others may operate correctly without it.
///
syntax snorm : SNormModifier;

/// Modifier to indicate that a function name should not be mangled
/// by the Slang compiler.
///
/// The `__extern_cpp` modifier makes a symbol to have unmangled
/// name in source/output C++ code.
///
syntax __extern_cpp : ExternCppModifier;

interface IComparable
{
    bool equals(This other);
    bool lessThan(This other);
    bool lessThanOrEquals(This other);
}

interface IRangedValue
{
    static const This maxValue;
    static const This minValue;
}

__attributeTarget(DeclBase)
attribute_syntax [TreatAsDifferentiable] : TreatAsDifferentiableAttribute;

interface IArithmetic : IComparable
{
    This add(This other);
    This sub(This other);
    This mul(This other);
    This div(This other);
    This mod(This other);
    This neg();

    __init(int val);

        /// Initialize from the same type.
    __init(This value);
}

interface ILogical : IComparable
{
    This shl(int value);
    This shr(int value);
    This bitAnd(This other);
    This bitOr(This other);
    This bitXor(This other);
    This bitNot();
    This and(This other);
    This or(This other);
    This not();
    __init(int val);
}

interface IInteger : IArithmetic, ILogical
{
    int toInt();
    int64_t toInt64();
    uint toUInt();
    uint64_t toUInt64();
}

interface IFloat : IArithmetic, IDifferentiable
{
    [TreatAsDifferentiable]
    __init(float value);

    [TreatAsDifferentiable]
    float toFloat();

    [TreatAsDifferentiable]
    This add(This other);

    [TreatAsDifferentiable]
    This sub(This other);

    [TreatAsDifferentiable]
    This mul(This other);

    [TreatAsDifferentiable]
    This div(This other);

    [TreatAsDifferentiable]
    This mod(This other);

    [TreatAsDifferentiable]
    This neg();

    [TreatAsDifferentiable]
    __init(This value);

    [TreatAsDifferentiable]
    This scale<T:__BuiltinFloatingPointType>(T scale);
}

/// A type that can be used as an operand for builtins
[sealed]
[builtin]
interface __BuiltinType {}

/// A type that can be used for arithmetic operations
[sealed]
[builtin]
interface __BuiltinArithmeticType : __BuiltinType, IArithmetic
{
}

/// A type that can be used for logical/bitwise operations
[sealed]
[builtin]
interface __BuiltinLogicalType : __BuiltinType, ILogical
{
}

/// A type that logically has a sign (positive/negative/zero)
[sealed]
[builtin]
interface __BuiltinSignedArithmeticType : __BuiltinArithmeticType {}

/// A type that can represent integers
[sealed]
[builtin]
interface __BuiltinIntegerType : __BuiltinArithmeticType
{}

/// A type that can represent non-integers
[sealed]
[builtin]
interface __BuiltinRealType : __BuiltinSignedArithmeticType {}


__attributeTarget(AggTypeDecl)
attribute_syntax [__NonCopyableType] : NonCopyableTypeAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [__NoSideEffect] : NoSideEffectAttribute;

/// Marks a function for forward-mode differentiation.
/// i.e. the compiler will automatically generate a new function
/// that computes the jacobian-vector product of the original.
__attributeTarget(FunctionDeclBase)
attribute_syntax [ForwardDifferentiable] : ForwardDifferentiableAttribute;

/// Marks a function for backward-mode differentiation.
__attributeTarget(FunctionDeclBase)
attribute_syntax [BackwardDifferentiable(order:int = 0)] : BackwardDifferentiableAttribute;
__attributeTarget(FunctionDeclBase)
attribute_syntax [Differentiable(order:int = 0)] : BackwardDifferentiableAttribute;

__intrinsic_op($(kIROp_RequirePrelude))
void __requirePrelude(constexpr String preludeText);

__intrinsic_op($(kIROp_RequireGLSLExtension))
void __requireGLSLExtension(constexpr String preludeText);

/// Interface to denote types as differentiable.
/// Allows for user-specified differential types as
/// well as automatic generation, for when the associated type
/// hasn't been declared explicitly.
/// Note that the requirements must currently be defined in this exact order
/// since the auto-diff pass relies on the order to grab the struct keys.
/// 
__magic_type(DifferentiableType)
interface IDifferentiable
{
    // Note: the compiler implementation requires the `Differential` associated type to be defined
    // before anything else.

    __builtin_requirement($( (int)BuiltinRequirementKind::DifferentialType) )
    associatedtype Differential : IDifferentiable;

    __builtin_requirement($( (int)BuiltinRequirementKind::DZeroFunc) )
    static Differential dzero();

    __builtin_requirement($( (int)BuiltinRequirementKind::DAddFunc) )
    static Differential dadd(Differential, Differential);

    __builtin_requirement($( (int)BuiltinRequirementKind::DMulFunc) )
    __generic<T : __BuiltinRealType>
    static Differential dmul(T, Differential);
};


/// Pair type that serves to wrap the primal and
/// differential types of an arbitrary type T.

__generic<T : IDifferentiable>
__magic_type(DifferentialPairType)
__intrinsic_type($(kIROp_DifferentialPairUserCodeType))
struct DifferentialPair : IDifferentiable
{
    typedef DifferentialPair<T.Differential> Differential;
    typedef T.Differential DifferentialElementType;

    __intrinsic_op($(kIROp_MakeDifferentialPairUserCode))
    __init(T _primal, T.Differential _differential);

    property p : T
    {
        __intrinsic_op($(kIROp_DifferentialPairGetPrimalUserCode))
        get;
    }

    property v : T
    {
        __intrinsic_op($(kIROp_DifferentialPairGetPrimalUserCode))
        get;
    }

    property d : T.Differential
    {
        __intrinsic_op($(kIROp_DifferentialPairGetDifferentialUserCode))
        get;
    }

    [__unsafeForceInlineEarly]
    T.Differential getDifferential()
    {
        return d;
    }

    [__unsafeForceInlineEarly]
    T getPrimal()
    {
        return p;
    }

    [__unsafeForceInlineEarly]
    static Differential dzero()
    {
        return Differential(T.dzero(), T.Differential.dzero());
    }

    [__unsafeForceInlineEarly]
    static Differential dadd(Differential a, Differential b)
    {
        return Differential(
            T.dadd(
                a.p,
                b.p
            ),
            T.Differential.dadd(a.d, b.d));
    }

    __generic<U : __BuiltinRealType>
    [__unsafeForceInlineEarly]
    static Differential dmul(U a, Differential b)
    {
        return Differential(
            T.dmul<U>(a, b.p),
            T.Differential.dmul<U>(a, b.d));
    }
};


/// A type that uses a floating-point representation
[sealed]
[builtin]
[TreatAsDifferentiable]
interface __BuiltinFloatingPointType : __BuiltinRealType, IFloat
{
        /// Get the value of the mathematical constant pi in this type.
    [Differentiable]
    static This getPi();
}

//@ hidden:

// A type resulting from an `enum` declaration.
[builtin]
__magic_type(EnumTypeType)
interface __EnumType
{
    // The type of tags for this `enum`
    //
    // Note: using `__Tag` instead of `Tag` to avoid any
    // conflict if a user had an `enum` case called `Tag`
    associatedtype __Tag : __BuiltinIntegerType;
};

// Use an extension to declare that every `enum` type
// inherits an initializer based on the tag type.
//
// Note: there is an important and subtle point here.
// If we declared these initializers inside the `interface`
// declaration above, then they would implicitly be
// *requirements* of the `__EnumType` interface, and any
// type that declares conformance to it would need to
// provide implementations. That would put the onus on
// the semantic checker to synthesize such initializers
// when conforming an `enum` type to `__EnumType` (just
// as it currently synthesizes the `__Tag` requirement.
// Putting the declaration in an `extension` makes them
// concrete declerations rather than interface requirements.
// (Admittedly, they are "concrete" declarations with
// no bodies, because currently all initializers are
// assumed to be intrinsics).
//
// TODO: It might be more accurate to express this as:
//
//      __generic<T:__EnumType> extension T { ... }
//
// That alternative would express an extension of every
// type that conforms to `__EnumType`, rather than an
// extension of `__EnumType` itself. The distinction
// is subtle, and unfortunately not one the Slang type
// checker is equiped to handle right now. For now we
// will stick with the syntax that actually works, even
// if it might be the less technically correct one.
//
//
extension __EnumType
{
    // TODO: this should be a single initializer using
    // the `__Tag` associated type from the `__EnumType`
    // interface, but right now the scoping for looking
    // up that type isn't working right.
    //
    __intrinsic_op($(kIROp_IntCast))
    __init(int value);
    __intrinsic_op($(kIROp_IntCast))
    __init(uint value);
}

// A type resulting from an `enum` declaration
// with the `[flags]` attribute.
[builtin]
interface __FlagsEnumType : __EnumType
{
};

interface IArray<T>
{
    int getCount();
    __subscript(int index) -> T
    {
        get;
    }
}

__generic<T, let N:int>
__magic_type(ArrayExpressionType)
struct Array : IArray<T>
{
    [ForceInline]
    int getCount() { return N; }

    __subscript(int index) -> T
    {
        __intrinsic_op($(kIROp_GetElement))
        get;
    }
}

// The "comma operator" is effectively just a generic function that returns its second
// argument. The left-to-right evaluation order guaranteed by Slang then ensures that
// `left` is evaluated before `right`.
//
__generic<T,U>
[__unsafeForceInlineEarly]
U operator,(T left, U right)
{
    return right;
}

// The ternary `?:` operator does not short-circuit in HLSL, and Slang no longer
// follow that definition for the scalar condition overload, so this declaration just serves
// for type-checking purpose only.

__generic<T> __intrinsic_op(select) T operator?:(bool condition, T ifTrue, T ifFalse);
__generic<T, let N : int> __intrinsic_op(select) vector<T,N> operator?:(vector<bool,N> condition, vector<T,N> ifTrue, vector<T,N> ifFalse);

// Users are advised to use `select` instead if non-short-circuiting behavior is intended.
__generic<T> __intrinsic_op(select) T select(bool condition, T ifTrue, T ifFalse);
__generic<T, let N : int> __intrinsic_op(select) vector<T,N> select(vector<bool,N> condition, vector<T,N> ifTrue, vector<T,N> ifFalse);

// Allow real-number types to be cast into each other
__intrinsic_op($(kIROp_FloatCast))
    T __realCast<T : __BuiltinRealType, U : __BuiltinRealType>(U val);
__intrinsic_op($(kIROp_IntCast))
    T __intCast<T : __BuiltinType, U : __BuiltinType>(U val);
${{{{
// We are going to use code generation to produce the
// declarations for all of our base types.
static const int kBaseTypeCount = sizeof(kBaseTypes) / sizeof(kBaseTypes[0]);
for (int tt = 0; tt < kBaseTypeCount; ++tt)
{
}}}}

__builtin_type($(int(kBaseTypes[tt].tag)))
struct $(kBaseTypes[tt].name)
    : __BuiltinType

${{{{
    switch (kBaseTypes[tt].tag)
    {
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
}}}}
    ,  __BuiltinFloatingPointType
    ,  __BuiltinRealType
    ,  __BuiltinSignedArithmeticType
    ,  __BuiltinArithmeticType
${{{{
        break;
    case BaseType::Int8:
    case BaseType::Int16:
    case BaseType::Int:
    case BaseType::Int64:
    case BaseType::IntPtr:
}}}}
    ,  __BuiltinSignedArithmeticType
${{{{
        ; // fall through
    case BaseType::UInt8:
    case BaseType::UInt16:
    case BaseType::UInt:
    case BaseType::UInt64:
    case BaseType::UIntPtr:
}}}}
    ,  __BuiltinArithmeticType
    ,  __BuiltinIntegerType
${{{{
        ; // fall through
    case BaseType::Bool:
}}}}
    ,  __BuiltinLogicalType
${{{{
        break;

    default:
        break;
    }
}}}}
{
${{{{
    // Declare initializers to convert from various other types
    for (int ss = 0; ss < kBaseTypeCount; ++ss)
    {
        // Don't allow conversion to or from `void`
        if (kBaseTypes[tt].tag == BaseType::Void)
            continue;
        if (kBaseTypes[ss].tag == BaseType::Void)
            continue;

        // We need to emit a modifier so that the semantic-checking
        // layer will know it can use these operations for implicit
        // conversion.
        ConversionCost conversionCost = getBaseTypeConversionCost(
            kBaseTypes[tt],
            kBaseTypes[ss]);

        IROp intrinsicOpCode = getBaseTypeConversionOp(
            kBaseTypes[tt],
            kBaseTypes[ss]);

        BuiltinConversionKind builtinConversionKind = kBuiltinConversion_Unknown;
        if (kBaseTypes[tt].tag == BaseType::Double &&
            kBaseTypes[ss].tag == BaseType::Float)
            builtinConversionKind = kBuiltinConversion_FloatToDouble;

        const char* attrib = "";
        if ((kBaseTypes[tt].flags & kBaseTypes[ss].flags & FLOAT_MASK) != 0)
            attrib = "[TreatAsDifferentiable]";
}}}}
    $(attrib)
    __intrinsic_op($(intrinsicOpCode))
    __implicit_conversion($(conversionCost), $(builtinConversionKind))
    __init($(kBaseTypes[ss].name) value);

${{{{
    }

    // Integer type implementations.
    switch (kBaseTypes[tt].tag)
    {
    case BaseType::Bool:
}}}}
        [__unsafeForceInlineEarly] __intrinsic_op($(kIROp_Eql)) bool equals(This other);
        [__unsafeForceInlineEarly] __intrinsic_op($(kIROp_Less)) bool lessThan(This other);
        [__unsafeForceInlineEarly] __intrinsic_op($(kIROp_Leq)) bool lessThanOrEquals(This other);
        [__unsafeForceInlineEarly] This shl(int other) { return __intCast<This>(__shl(__intCast<int>(this), other)); }
        [__unsafeForceInlineEarly] This shr(int other) { return __intCast<This>(__shr(__intCast<int>(this), other)); }
        [__unsafeForceInlineEarly] This bitAnd(This other) { return __intCast<This>(__and(__intCast<int>(this), __intCast<int>(other))); }
        [__unsafeForceInlineEarly] This bitOr(This other) { return __intCast<This>(__or(__intCast<int>(this), __intCast<int>(other))); }
        [__unsafeForceInlineEarly] __intrinsic_op($(kIROp_And)) This and(This other);
        [__unsafeForceInlineEarly] __intrinsic_op($(kIROp_Or)) This or(This other);
        [__unsafeForceInlineEarly] This bitXor(This other)  { return __intCast<This>(__xor(__intCast<int>(this), __intCast<int>(other))); }
        [__unsafeForceInlineEarly] This bitNot()  { return __intCast<This>(__not(__intCast<int>(this))); }
        [__unsafeForceInlineEarly] __intrinsic_op($(kIROp_Not)) This not();
${{{{
        break;
    case BaseType::UInt8:
    case BaseType::UInt16:
    case BaseType::UInt:
    case BaseType::UInt64:
    case BaseType::Int8:
    case BaseType::Int16:
    case BaseType::Int:
    case BaseType::Int64:
    case BaseType::IntPtr:
    case BaseType::UIntPtr:
}}}}
        // If this is a basic integer type, then define explicit
        // initializers that take a value of an `enum` type.
        //
        // TODO: This should actually be restricted, so that this
        // only applies `where T.__Tag == Self`, but we don't have
        // the needed features in our type system to implement
        // that constraint right now.
        //
        __generic<T:__EnumType>
        __intrinsic_op($(kIROp_IntCast))
        __init(T value);

        // Implementation of the `IInteger` interface.
        __intrinsic_op($(kIROp_Less)) bool lessThan(This other);
        __intrinsic_op($(kIROp_Leq))  bool lessThanOrEquals(This other);
        __intrinsic_op($(kIROp_Eql))  bool equals(This other);
        __intrinsic_op($(kIROp_Add))  This add(This other);
        __intrinsic_op($(kIROp_Sub))  This sub(This other);
        __intrinsic_op($(kIROp_Mul))  This mul(This other);
        __intrinsic_op($(kIROp_Div))  This div(This other);
        __intrinsic_op($(kIROp_FRem)) This mod(This other);
        __intrinsic_op($(kIROp_Neg))  This neg();
        __intrinsic_op($(kIROp_Lsh)) This shl(int other);
        __intrinsic_op($(kIROp_Rsh)) This shr(int other);
        __intrinsic_op($(kIROp_BitAnd)) This bitAnd(This other);
        __intrinsic_op($(kIROp_BitOr)) This bitOr(This other);
        [__unsafeForceInlineEarly] This and(This other) {return __intCast<This>(__intCast<bool>(this) && __intCast<bool>(other)); }
        [__unsafeForceInlineEarly] This or(This other) {return __intCast<This>(__intCast<bool>(this) || __intCast<bool>(other)); }
        __intrinsic_op($(kIROp_BitXor)) This bitXor(This other);
        __intrinsic_op($(kIROp_BitNot)) This bitNot();
        [__unsafeForceInlineEarly] This not() {return __intCast<This>(!__intCast<bool>(this)); }
        __intrinsic_op($(kIROp_IntCast)) int toInt();
        __intrinsic_op($(kIROp_IntCast)) int64_t toInt64();
        __intrinsic_op($(kIROp_IntCast)) uint toUInt();
        __intrinsic_op($(kIROp_IntCast)) uint64_t toUInt64();
${{{{
        break;

    default:
        break;
    }

    // If this is a floating-point type, then we need to
    // implement the `IFloat` interface, which defines the basic `getPi()`
    // function that is used to implement generic versions of `degrees()` and
    // `radians()`.
    //
    switch (kBaseTypes[tt].tag)
    {
    default:
        break;
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
}}}}
        [Differentiable]
        static $(kBaseTypes[tt].name) getPi() { return $(kBaseTypes[tt].name)(3.14159265358979323846264338328); }

         __intrinsic_op($(kIROp_Less)) bool lessThan(This other);
         __intrinsic_op($(kIROp_Leq))  bool lessThanOrEquals(This other);
         __intrinsic_op($(kIROp_Eql))  bool equals(This other);
         __intrinsic_op($(kIROp_Add))  This add(This other);
         __intrinsic_op($(kIROp_Sub))  This sub(This other);
         __intrinsic_op($(kIROp_Mul))  This mul(This other);
         __intrinsic_op($(kIROp_Div))  This div(This other);
         __intrinsic_op($(kIROp_FRem)) This mod(This other);
         __intrinsic_op($(kIROp_Neg))  This neg();
         __intrinsic_op($(kIROp_FloatCast)) float toFloat();
         __intrinsic_op($(kIROp_Mul)) This scale<T:__BuiltinFloatingPointType>(T s) { return __mul(this, __realCast<This>(s)); }
        typedef $(kBaseTypes[tt].name) Differential;
    
        [__unsafeForceInlineEarly]
        [BackwardDifferentiable]
        static Differential dzero()
        {
            return Differential(0);
        }

        [__unsafeForceInlineEarly]
        [BackwardDifferentiable]
        static Differential dadd(Differential a, Differential b)
        {
            return a + b;
        }
        
        __generic<U : __BuiltinRealType>
        [__unsafeForceInlineEarly]
        [BackwardDifferentiable]
        static Differential dmul(U a, Differential b)
        {
            return __realCast<Differential, U>(a) * b;
        }
${{{{
        break;
    }

    // If this is the `void` type, then we want to allow
    // explicit conversion to it from any other type, using
    // `(void) someExpression`.
    //
    if( kBaseTypes[tt].tag == BaseType::Void )
    {
}}}}
        __generic<T>
        [__readNone]
        __intrinsic_op($(kIROp_CastToVoid))
        __init(T value)
        {}
${{{{
    }

}}}}

}

${{{{
}

// Declare built-in pointer type
// (eventually we can have the traditional syntax sugar for this)
}}}}

__magic_type(NullPtrType)
struct NullPtr
{
};

__magic_type(NoneType)
__intrinsic_type($(kIROp_VoidType))
struct __none_t
{
};

__generic<T>
__magic_type(PtrType)
__intrinsic_type($(kIROp_PtrType))
struct Ptr
{
    __generic<U>
    __intrinsic_op($(kIROp_BitCast))
    __init(Ptr<U> ptr);

    __intrinsic_op($(kIROp_CastIntToPtr))
    __init(uint64_t val);

    __intrinsic_op($(kIROp_CastIntToPtr))
    __init(int64_t val);

    __subscript(int index) -> T
    {
        __intrinsic_op($(kIROp_GetOffsetPtr))
        ref;
    }
};

__intrinsic_op($(kIROp_Load))
T __load<T>(Ptr<T> ptr);

__intrinsic_op($(kIROp_Store))
void __store<T>(Ptr<T> ptr, T val);

__intrinsic_op($(kIROp_GetElementPtr))
Ptr<T> __getElementPtr<T>(Ptr<T> ptr, int index);

__intrinsic_op($(kIROp_GetElementPtr))
Ptr<T> __getElementPtr<T>(Ptr<T> ptr, int64_t index);

__intrinsic_op($(kIROp_GetOffsetPtr))
Ptr<T> __getOffsetPtr<T>(Ptr<T> ptr, int index);

__intrinsic_op($(kIROp_GetOffsetPtr))
Ptr<T> __getOffsetPtr<T>(Ptr<T> ptr, int64_t index);

__generic<T>
__intrinsic_op($(kIROp_Less))
bool operator<(Ptr<T> p1, Ptr<T> p2);

__generic<T>
__intrinsic_op($(kIROp_Leq))
bool operator<=(Ptr<T> p1, Ptr<T> p2);

__generic<T>
__intrinsic_op($(kIROp_Greater))
bool operator>(Ptr<T> p1, Ptr<T> p2);

__generic<T>
__intrinsic_op($(kIROp_Geq))
bool operator>=(Ptr<T> p1, Ptr<T> p2);

__generic<T>
__intrinsic_op($(kIROp_Neq))
bool operator!=(Ptr<T> p1, Ptr<T> p2);

__generic<T>
__intrinsic_op($(kIROp_Eql))
bool operator==(Ptr<T> p1, Ptr<T> p2);

extension bool : IRangedValue
{
    __generic<T>
    __implicit_conversion($(kConversionCost_PtrToBool))
    __intrinsic_op($(kIROp_CastPtrToBool))
    __init(Ptr<T> ptr);

    static const bool maxValue = true;
    static const bool minValue = false;
}

extension uint64_t : IRangedValue
{
    __generic<T>
    __intrinsic_op($(kIROp_CastPtrToInt))
    __init(Ptr<T> ptr);

    static const uint64_t maxValue = 0xFFFFFFFFFFFFFFFFULL;
    static const uint64_t minValue = 0;
}

extension int64_t : IRangedValue
{
    __generic<T>
    __intrinsic_op($(kIROp_CastPtrToInt))
    __init(Ptr<T> ptr);

    static const int64_t maxValue =  0x7FFFFFFFFFFFFFFFLL;
    static const int64_t minValue = -0x8000000000000000LL;
}

extension intptr_t : IRangedValue
{
    __generic<T>
    __intrinsic_op($(kIROp_CastPtrToInt))
    __init(Ptr<T> ptr);
    static const intptr_t maxValue = $(SLANG_PROCESSOR_X86_64?"0x7FFFFFFFFFFFFFFFz":"0x7FFFFFFFz");
    static const intptr_t minValue = $(SLANG_PROCESSOR_X86_64?"0x8000000000000000z":"0x80000000z");
    static const int size = $(SLANG_PROCESSOR_X86_64?"8":"4");
}

extension uintptr_t : IRangedValue
{
    __generic<T>
    __intrinsic_op($(kIROp_CastPtrToInt))
    __init(Ptr<T> ptr);
    static const uintptr_t maxValue = $(SLANG_PROCESSOR_X86_64?"0xFFFFFFFFFFFFFFFFz":"0xFFFFFFFFz");
    static const uintptr_t minValue = 0z;
    static const int size = $(SLANG_PROCESSOR_X86_64?"8":"4");
}

__generic<T>
__magic_type(OutType)
__intrinsic_type($(kIROp_OutType))
struct Out
{};

__generic<T>
__magic_type(InOutType)
__intrinsic_type($(kIROp_InOutType))
struct InOut
{};

__generic<T>
__magic_type(RefType)
__intrinsic_type($(kIROp_RefType))
struct Ref
{};

__generic<T>
__magic_type(ConstRefType)
__intrinsic_type($(kIROp_ConstRefType))
struct ConstRef
{};

__generic<T>
__magic_type(OptionalType)
__intrinsic_type($(kIROp_OptionalType))
struct Optional
{
    property bool hasValue
    {
        __intrinsic_op($(kIROp_OptionalHasValue))
        get;
    }

    property T value
    {
        __intrinsic_op($(kIROp_GetOptionalValue))
        get;
    }

    __implicit_conversion($(kConversionCost_ValToOptional))
    __intrinsic_op($(kIROp_MakeOptionalValue))
    __init(T val);
};

__generic<T>
[__unsafeForceInlineEarly]
bool operator==(Optional<T> val, __none_t noneVal)
{
    return !val.hasValue;
}
__generic<T>
[__unsafeForceInlineEarly]
bool operator!=(Optional<T> val, __none_t noneVal)
{
    return val.hasValue;
}
__generic<T>
[__unsafeForceInlineEarly]
bool operator==(__none_t noneVal, Optional<T> val)
{
    return !val.hasValue;
}
__generic<T>
[__unsafeForceInlineEarly]
bool operator!=(__none_t noneVal, Optional<T> val)
{
    return val.hasValue;
}

__generic<T>
__magic_type(NativeRefType)
__intrinsic_type($(kIROp_NativePtrType))
struct NativeRef
{
    __intrinsic_op($(kIROp_GetNativePtr))
    __init(T val);
};

__generic<T>
__intrinsic_op($(kIROp_ManagedPtrAttach))
void __managed_ptr_attach(__ref T val, NativeRef<T> nativeVal);

__generic<T>
[__unsafeForceInlineEarly]
T __attachToNativeRef(NativeRef<T> nativeVal)
{
    T result;
    __managed_ptr_attach(result, nativeVal);
    return result;
}

__magic_type(StringType)
__intrinsic_type($(kIROp_StringType))
struct String
{
    __target_intrinsic(cpp)
    __intrinsic_op($(kIROp_MakeString))
    __init(int val);
    __target_intrinsic(cpp)
    __intrinsic_op($(kIROp_MakeString))
    __init(uint val);
    __target_intrinsic(cpp)
    __intrinsic_op($(kIROp_MakeString))
    __init(int64_t val);
    __target_intrinsic(cpp)
    __intrinsic_op($(kIROp_MakeString))
    __init(uint64_t val);
    __target_intrinsic(cpp)
    __intrinsic_op($(kIROp_MakeString))
    __init(float val);
    __target_intrinsic(cpp)
    __intrinsic_op($(kIROp_MakeString))
    __init(double val);

    __target_intrinsic(cpp)
    int64_t getLength();

    property int length
    {
        get { return (int)getLength(); }
    }
};

typedef String string;

__magic_type(NativeStringType)
__intrinsic_type($(kIROp_NativeStringType))
struct NativeString
{
    __target_intrinsic(cpp, "int(strlen($0))")
    int getLength();

    __target_intrinsic(cpp, "(void*)((const char*)($0))")
    Ptr<void> getBuffer();

    property int length { [__unsafeForceInlineEarly] get{return getLength();} }

    __intrinsic_op($(kIROp_getNativeStr))
    __init(String value);
};

extension Ptr<void>
{
    __implicit_conversion($(kConversionCost_PtrToVoidPtr))
    [__unsafeForceInlineEarly]
    __init(NativeString nativeStr) { this = nativeStr.getBuffer(); }

    __generic<T>
    __intrinsic_op(0)
    __implicit_conversion($(kConversionCost_PtrToVoidPtr))
    __init(Ptr<T> ptr);

    __generic<T>
    __intrinsic_op(0)
    __implicit_conversion($(kConversionCost_PtrToVoidPtr))
    __init(NativeRef<T> ptr);
}

__magic_type(DynamicType)
__intrinsic_type($(kIROp_DynamicType))
struct __Dynamic
{};

extension half : IRangedValue
{
    static const half maxValue = half(65504);
    static const half minValue = half(-65504);
}

extension float : IRangedValue
{
    static const float maxValue = 340282346638528859811704183484516925440.0f;
    static const float minValue = -340282346638528859811704183484516925440.0f;

}

extension double : IRangedValue
{
    static const double maxValue = bit_cast<double>(0x7fefffffffffffffULL);
    static const double minValue = bit_cast<double>(0xffefffffffffffffULL);
}

extension int : IRangedValue
{
    static const int maxValue = 2147483647;
    static const int minValue = -2147483648;
}

extension uint : IRangedValue
{
    static const uint maxValue = 4294967295;
    static const uint minValue = 0;
}

extension int8_t : IRangedValue
{
    static const int8_t maxValue = 127;
    static const int8_t minValue = -128;
}

extension uint8_t : IRangedValue
{
    static const uint8_t maxValue = 255;
    static const uint8_t minValue = 0;
}

extension uint16_t : IRangedValue
{
    static const uint16_t maxValue = 65535;
    static const uint16_t minValue = 0;
}

extension int16_t : IRangedValue
{
    static const int16_t maxValue = 32767;
    static const int16_t minValue = -32768;
}

    /// An `N` component vector with elements of type `T`.
__generic<T = float, let N : int = 4>
__magic_type(VectorExpressionType)
struct vector : IArray<T>
{
        /// The element type of the vector
    typedef T Element;


        /// Initialize a vector where all elements have the same scalar `value`.
    [TreatAsDifferentiable]
    __implicit_conversion($(kConversionCost_ScalarToVector))
    __intrinsic_op($(kIROp_MakeVectorFromScalar))
    __init(T value);

    /// Initialize a vector from a value of the same type
    // TODO: we should revise semantic checking so this kind of "identity" conversion is not required
    __intrinsic_op(0)
    [TreatAsDifferentiable]
    __init(vector<T,N> value);

    [ForceInline]
    int getCount() { return N; }

    __subscript(int index) -> T { __intrinsic_op($(kIROp_GetElement)) get; }
}

const int kRowMajorMatrixLayout = $(SLANG_MATRIX_LAYOUT_ROW_MAJOR);
const int kColumnMajorMatrixLayout = $(SLANG_MATRIX_LAYOUT_COLUMN_MAJOR);

    /// A matrix with `R` rows and `C` columns, with elements of type `T`.
__generic<T = float, let R : int = 4, let C : int = 4, let L : int = $(SLANG_MATRIX_LAYOUT_MODE_UNKNOWN)>
__magic_type(MatrixExpressionType)
struct matrix : IArray<vector<T,C>>
{
    __intrinsic_op($(kIROp_MakeMatrixFromScalar))
    [TreatAsDifferentiable]
    __init(T val);

    /// Initialize a vector from a value of the same type
    // TODO: we should revise semantic checking so this kind of "identity" conversion is not required
    __intrinsic_op(0)
    [TreatAsDifferentiable]
    __init(This value);

    [ForceInline]
    int getCount() { return R; }

    __subscript(int index) -> vector<T,C> { __intrinsic_op($(kIROp_GetElement)) get; }
}

__intrinsic_op($(kIROp_Eql))
vector<bool, N> __vectorEql<T, let N:int>(vector<T, N> left, vector<T,N> right);

__generic<T:__BuiltinFloatingPointType, let N : int>
extension vector<T,N> : IFloat
{
    [__unsafeForceInlineEarly] bool lessThan(This other) { return this[0] < other[0]; }
    [__unsafeForceInlineEarly] bool lessThanOrEquals(This other) { return this[0] <= other[0]; }
    [__unsafeForceInlineEarly] bool equals(This other) { return all(__vectorEql(this, other)); }
    __intrinsic_op($(kIROp_Add)) This add(This other);
    __intrinsic_op($(kIROp_Sub)) This sub(This other);
    __intrinsic_op($(kIROp_Mul)) This mul(This other);
    __intrinsic_op($(kIROp_Div)) This div(This other);
    __intrinsic_op($(kIROp_FRem)) This mod(This other);
    __intrinsic_op($(kIROp_Neg)) This neg();
    __intrinsic_op($(kIROp_Mul)) This scale<T1:__BuiltinFloatingPointType>(T1 s);
    [__unsafeForceInlineEarly] float toFloat() { return __realCast<float>(this[0]); }

    [OverloadRank(-1)]
    [__unsafeForceInlineEarly] __init(int v) { return vector<T,N>(T(v)); }
    [OverloadRank(-1)]
    [__unsafeForceInlineEarly] __init(float v) { return vector<T,N>(T(v)); }

    // IDifferentiable

    typedef vector<T, N> Differential;

    [__unsafeForceInlineEarly]
    [BackwardDifferentiable]
    static Differential dzero()
    {
        return Differential(__slang_noop_cast<T>(T.dzero()));
    }

    [__unsafeForceInlineEarly]
    [BackwardDifferentiable]
    static Differential dadd(Differential a, Differential b)
    {
        return a + b;
    }

    __generic<U : __BuiltinRealType>
    [__unsafeForceInlineEarly]
    [BackwardDifferentiable]
    static Differential dmul(U a, Differential b)
    {
        return __realCast<T, U>(a) * b;
    }
}

__generic<T:__BuiltinFloatingPointType, let N : int, let M : int, let L : int>
extension matrix<T,N,M,L> : IFloat
{
    [TreatAsDifferentiable][__unsafeForceInlineEarly] bool lessThan(This other) { return this < other; }
    [TreatAsDifferentiable][__unsafeForceInlineEarly] bool lessThanOrEquals(This other) { return this <= other; }
    [TreatAsDifferentiable][__unsafeForceInlineEarly] bool equals(This other) { return all(this == other); }
    [TreatAsDifferentiable] __intrinsic_op($(kIROp_Add)) This add(This other);
    [TreatAsDifferentiable] __intrinsic_op($(kIROp_Sub)) This sub(This other);
    [TreatAsDifferentiable] __intrinsic_op($(kIROp_Mul))This mul(This other);
    [TreatAsDifferentiable] __intrinsic_op($(kIROp_Div)) This div(This other);
    [TreatAsDifferentiable] __intrinsic_op($(kIROp_FRem)) This mod(This other);
    [TreatAsDifferentiable] __intrinsic_op($(kIROp_Neg)) This neg();
    [TreatAsDifferentiable] __intrinsic_op($(kIROp_Mul)) This scale<T1:__BuiltinFloatingPointType>(T1 s);
    [TreatAsDifferentiable][__unsafeForceInlineEarly] This scale<T1:__BuiltinFloatingPointType>(T1 s);
    [TreatAsDifferentiable][__unsafeForceInlineEarly] float toFloat() { return __realCast<float>(this[0][0]); }

    [OverloadRank(-1)]
    [TreatAsDifferentiable][__unsafeForceInlineEarly] __init(int v) { return matrix<T,N,M>(T(v)); }
    [OverloadRank(-1)]
    [TreatAsDifferentiable][__unsafeForceInlineEarly] __init(float v) { return matrix<T,N,M>(T(v)); }

    // IDifferentiable.
    typedef matrix<T, N,M,L> Differential;

    [__unsafeForceInlineEarly]
    [BackwardDifferentiable]
    static Differential dzero()
    {
        return matrix<T, N,M,L>(__slang_noop_cast<T>(T.dzero()));
    }

    [__unsafeForceInlineEarly]
    [BackwardDifferentiable]
    static Differential dadd(Differential a, Differential b)
    {
        return a + b;
    }
    
    __generic<U : __BuiltinRealType>
    [__unsafeForceInlineEarly]
    [BackwardDifferentiable]
    static Differential dmul(U a, Differential b)
    {
        return __realCast<T, U>(a) * b;
    }
}

${{{{
static const struct {
    char const* name;
    char const* glslPrefix;
} kTypes[] =
{
    {"half",        "f16"},
    {"float",       ""},
    {"double",      "d"},

    {"float16_t",   "f16"},
    {"float32_t",   "f32"},
    {"float64_t",   "f64"},

    {"int8_t",      "i8"},
    {"int16_t",     "i16"},
    {"int32_t",     "i32"},
    {"int",         "i"},
    {"int64_t",     "i64"},

    {"uint8_t",     "u8"},
    {"uint16_t",    "u16"},
    {"uint32_t",    "u32"},
    {"uint",        "u"},
    {"uint64_t",    "u64"},

    {"bool",        "b"},
};

static const int kTypeCount = sizeof(kTypes) / sizeof(kTypes[0]);

for (int tt = 0; tt < kTypeCount; ++tt)
{
    // Declare HLSL vector types
    for (int ii = 1; ii <= 4; ++ii)
    {
        sb << "typedef vector<" << kTypes[tt].name << "," << ii << "> " << kTypes[tt].name << ii << ";\n";
    }

    // Declare HLSL matrix types
    for (int rr = 2; rr <= 4; ++rr)
    for (int cc = 2; cc <= 4; ++cc)
    {
        sb << "typedef matrix<" << kTypes[tt].name << "," << rr << "," << cc << "> " << kTypes[tt].name << rr << "x" << cc << ";\n";
    }
}

// Declare additional built-in generic types
}}}}

//@ public:

__generic<T>
__intrinsic_type($(kIROp_ConstantBufferType))
__magic_type(ConstantBufferType)
struct ConstantBuffer {}

__generic<T>
__intrinsic_type($(kIROp_TextureBufferType))
__magic_type(TextureBufferType)
struct TextureBuffer {}

__generic<T>
__intrinsic_type($(kIROp_ParameterBlockType))
__magic_type(ParameterBlockType)
struct ParameterBlock {}

__generic<T, let MAX_VERTS : uint>
__magic_type(VerticesType)
__intrinsic_type($(kIROp_VerticesType))
struct Vertices
{
    __subscript(uint index) -> T
    {
        // TODO: Ellie make sure these remains write only
        __intrinsic_op($(kIROp_GetElementPtr))
        ref;
    }
};

__generic<T, let MAX_PRIMITIVES : uint>
__magic_type(IndicesType)
__intrinsic_type($(kIROp_IndicesType))
struct Indices
{
    __subscript(uint index) -> T
    {
        // TODO: Ellie: It's illegal to not write out the whole primitive at once, should we use set over ref?
        __intrinsic_op($(kIROp_GetElementPtr))
        ref;
    }
};

__generic<T, let MAX_PRIMITIVES : uint>
__magic_type(PrimitivesType)
__intrinsic_type($(kIROp_PrimitivesType))
struct Primitives
{
    __subscript(uint index) -> T
    {
        __intrinsic_op($(kIROp_GetElementPtr))
        ref;
    }
};

//@ hidden:

// Need to add constructors to the types above

__generic<T> __extension vector<T, 2>
{
    __intrinsic_op($(kIROp_MakeVector))
    __init(T x, T y);
}
__generic<T> __extension vector<T, 3>
{
    __intrinsic_op($(kIROp_MakeVector))
    __init(T x, T y, T z);

    [__unsafeForceInlineEarly]
    __intrinsic_op($(kIROp_MakeVector))
    __init(vector<T,2> xy, T z);

    [__unsafeForceInlineEarly]
    __intrinsic_op($(kIROp_MakeVector))
    __init(T x, vector<T,2> yz);
}
__generic<T> __extension vector<T, 4>
{
    __intrinsic_op($(kIROp_MakeVector))
    __init(T x, T y, T z, T w);

    [__unsafeForceInlineEarly]
    __intrinsic_op($(kIROp_MakeVector))
    __init(vector<T,2> xy, T z, T w);

    [__unsafeForceInlineEarly]
    __intrinsic_op($(kIROp_MakeVector))
    __init(T x, vector<T,2> yz, T w);

    [__unsafeForceInlineEarly]
    __intrinsic_op($(kIROp_MakeVector))
    __init(T x, T y, vector<T,2> zw);

    [__unsafeForceInlineEarly]
    __intrinsic_op($(kIROp_MakeVector))
    __init(vector<T,2> xy, vector<T,2> zw);

    [__unsafeForceInlineEarly]
    __intrinsic_op($(kIROp_MakeVector))
    __init(vector<T,3> xyz, T w);

    [__unsafeForceInlineEarly]
    __intrinsic_op($(kIROp_MakeVector))
    __init(T x, vector<T,3> yzw);
}

${{{{

// The above extensions are generic in the *type* of the vector,
// but explicit in the *size*. We will now declare an extension
// for each builtin type that is generic in the size.
//
for (int tt = 0; tt < kBaseTypeCount; ++tt)
{
    if(kBaseTypes[tt].tag == BaseType::Void) continue;

    sb << "__generic<let N : int> __extension vector<"
        << kBaseTypes[tt].name << ",N>\n{\n";

    for (int ff = 0; ff < kBaseTypeCount; ++ff)
    {
        if(kBaseTypes[ff].tag == BaseType::Void) continue;


        if( tt != ff )
        {
            auto cost = getBaseTypeConversionCost(
                kBaseTypes[tt],
                kBaseTypes[ff]);
            auto op = getBaseTypeConversionOp(
                kBaseTypes[tt],
                kBaseTypes[ff]);

			// Implicit conversion from a vector of the same
			// size, but different element type.
            sb << "    __implicit_conversion(" << cost << ")\n";
            sb << "    __intrinsic_op(" << int(op) << ")\n";
            sb << "    __init(vector<" << kBaseTypes[ff].name << ",N> value);\n";

			// Constructor to make a vector from a scalar of another type.
            if (cost != kConversionCost_Impossible)
            {
                cost += kConversionCost_ScalarToVector;
                sb << "    __implicit_conversion(" << cost << ")\n";
                sb << "    [__unsafeForceInlineEarly]\n";
                sb << "    __init(" << kBaseTypes[ff].name << " value) { this = vector<" << kBaseTypes[tt].name << ",N>( " << kBaseTypes[tt].name << "(value)); }\n";
            }
        }
    }
    sb << "}\n";
}

for( int R = 2; R <= 4; ++R )
for( int C = 2; C <= 4; ++C )
{
    sb << "__generic<T, let L:int> __extension matrix<T, " << R << "," << C << ", L>\n{\n";

    // initialize from R*C scalars
    sb << "__intrinsic_op(" << int(kIROp_MakeMatrix) << ") __init(";
    for( int ii = 0; ii < R; ++ii )
    for( int jj = 0; jj < C; ++jj )
    {
        if ((ii+jj) != 0) sb << ", ";
        sb << "T m" << ii << jj;
    }
    sb << ");\n";

    // Initialize from R C-vectors
    sb << "__intrinsic_op(" << int(kIROp_MakeMatrix) << ") __init(";
    for (int ii = 0; ii < R; ++ii)
    {
        if(ii != 0) sb << ", ";
        sb << "vector<T," << C << "> row" << ii;
    }
    sb << ");\n";

    // initialize from a matrix of larger size
    for(int rr = R; rr <= 4; ++rr)
    for( int cc = C; cc <= 4; ++cc )
    {
        if(rr == R && cc == C) continue;
        sb << "__intrinsic_op(" << int(kIROp_MatrixReshape) << ") __init(matrix<T," << rr << "," << cc << ", L> value);\n";
    }
    sb << "}\n";
}

for (int tt = 0; tt < kBaseTypeCount; ++tt)
{
    if(kBaseTypes[tt].tag == BaseType::Void) continue;
    auto toType = kBaseTypes[tt].name;
}}}}

__generic<let R : int, let C : int, let L : int> extension matrix<$(toType),R,C,L>
{
${{{{
    for (int ff = 0; ff < kBaseTypeCount; ++ff)
    {
        if(kBaseTypes[ff].tag == BaseType::Void) continue;
        if( tt == ff ) continue;

        auto cost = getBaseTypeConversionCost(
            kBaseTypes[tt],
            kBaseTypes[ff]);
        auto fromType = kBaseTypes[ff].name;
        auto op = getBaseTypeConversionOp(
                kBaseTypes[tt],
                kBaseTypes[ff]);
}}}}
    __implicit_conversion($(cost))
    __intrinsic_op($(op))
    __init(matrix<$(fromType),R,C,L> value);
${{{{
    }
}}}}
}
${{{{
}
}}}}

__generic<T, U>
__intrinsic_op(0)
T __slang_noop_cast(U u);


//@ public:

    /// Sampling state for filtered texture fetches.
__magic_type(SamplerStateType, $(int(SamplerStateFlavor::SamplerState)))
__intrinsic_type($(kIROp_SamplerStateType))
struct SamplerState
{
}

    /// Sampling state for filtered texture fetches that include a comparison operation before filtering.
__magic_type(SamplerStateType, $(int(SamplerStateFlavor::SamplerComparisonState)))
__intrinsic_type($(kIROp_SamplerComparisonStateType))
struct SamplerComparisonState
{
}

//@ hidden:

${{{{

for (auto op : intrinsicUnaryOps)
{
    for (auto type : kBaseTypes)
    {
        if ((type.flags & op.flags) == 0)
            continue;

        char const* resultType = type.name;
        if (op.flags & BOOL_RESULT) resultType = "bool";

        // scalar version
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << type.name << " value);\n";
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " __" << op.funcName << "(" << type.name << " value);\n";

        // vector version
        sb << "__generic<let N : int> ";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << "vector<" << type.name << ",N> value);\n";

        // matrix version
        sb << "__generic<let N : int, let M : int> ";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" <<  "matrix<" << type.name << ",N,M> value);\n";
    }

    // Synthesize generic versions
    if(op.interface)
    {
        char const* resultType = "T";
        if (op.flags & BOOL_RESULT) resultType = "bool";
        
        // scalar version
        sb << "__generic<T : " << op.interface << ">\n";
        sb << "[OverloadRank(10)]";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << "T value);\n";

        // vector version
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<T,N> value);\n";

        // matrix version
        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<T,N,M> value);\n";
    }
}

}}}}

__generic<T>
__intrinsic_op(0)
__prefix Ref<T> operator*(Ptr<T> value);

__generic<T>
__intrinsic_op(0)
__prefix Ptr<T> operator&(__ref T value);

__generic<T>
__intrinsic_op($(kIROp_GetOffsetPtr))
Ptr<T> operator+(Ptr<T> value, int64_t offset);

__generic<T>
[__unsafeForceInlineEarly]
Ptr<T> operator-(Ptr<T> value, int64_t offset)
{
    return __getOffsetPtr(value, -offset);
}

__generic<T : IArithmetic>
[__unsafeForceInlineEarly]
__prefix T operator+(T value)
{ return value; }

__generic<T : __BuiltinArithmeticType, let N : int>
[__unsafeForceInlineEarly]
__prefix vector<T,N> operator+(vector<T,N> value)
{ return value; }

__generic<T : __BuiltinArithmeticType, let R : int, let C : int>
[__unsafeForceInlineEarly]
__prefix matrix<T,R,C> operator+(matrix<T,R,C> value)
{ return value; }

${{{{

static const struct IncDecOpInfo
{
    char const* name;
    char const* binOp;
} kIncDecOps[] =
{
    { "++", "+" },
    { "--", "-" },
};
static const struct IncDecOpFixity
{
    char const* qual;
    char const* bodyPrefix;
    char const* returnVal;
} kIncDecFixities[] =
{
    { "__prefix", "", "value" },
    { "__postfix", " let result = value;", "result" },
};
for(auto op : kIncDecOps)
for(auto fixity : kIncDecFixities)
{
}}}}

$(fixity.qual)
__generic<T : __BuiltinArithmeticType>
[__unsafeForceInlineEarly]
T operator$(op.name)(in out T value)
{$(fixity.bodyPrefix) value = value $(op.binOp) T(1); return $(fixity.returnVal); }

$(fixity.qual)
__generic<T : __BuiltinArithmeticType, let N : int>
[__unsafeForceInlineEarly]
vector<T,N> operator$(op.name)(in out vector<T,N> value)
{$(fixity.bodyPrefix) value = value $(op.binOp) T(1); return $(fixity.returnVal); }

$(fixity.qual)
__generic<T : __BuiltinArithmeticType, let R : int, let C : int, let L : int>
[__unsafeForceInlineEarly]
matrix<T,R,C> operator$(op.name)(in out matrix<T,R,C,L> value)
{$(fixity.bodyPrefix) value = value $(op.binOp) T(1); return $(fixity.returnVal); }

$(fixity.qual)
__generic<T>
[__unsafeForceInlineEarly]
Ptr<T> operator$(op.name)(in out Ptr<T> value)
{$(fixity.bodyPrefix) value = value $(op.binOp) 1; return $(fixity.returnVal); }

${{{{
}

for (auto op : intrinsicBinaryOps)
{
    for (auto type : kBaseTypes)
    {
        if ((type.flags & op.flags) == 0)
            continue;

        char const* leftType = type.name;
        char const* rightType = leftType;
        char const* resultType = leftType;

        if (op.flags & BOOL_RESULT) resultType = "bool";

        // TODO: We should handle a `SHIFT` flag on the op
        // by changing `rightType` to `int` in order to
        // account for the fact that the shift amount should
        // always have a fixed type independent of the LHS.
        //
        // (It is unclear why this change hadn't been made
        // already, so it is possible that such a change
        // breaks overload resolution or other parts of
        // the compiler)

        // scalar version
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << leftType << " left, " << rightType << " right);\n";
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " __" << op.funcName << "(" << leftType << " left, " << rightType << " right);\n";

        // vector version
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, vector<" << rightType << ",N> right);\n";

        // matrix version
        sb << "__generic<let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, matrix<" << rightType << ",N,M> right);\n";

        // We currently synthesize addiitonal overloads
        // for the case where one or the other operand
        // is a scalar. This choice serves a few purposes:
        //
        // 1. It avoids introducing scalar-to-vector or
        // scalar-to-matrix promotions before the operator,
        // which might allow some back ends to produce
        // more optimal code.
        //
        // 2. It avoids concerns about making overload resolution
        // and the inference rules for `N` and `M` able to
        // handle the mixed vector/scalar or matrix/scalar case.
        //
        // 3. Having explicit overloads for the matrix/scalar cases
        // here means that we do *not* need to support a general
        // implicit conversion from scalars to matrices, unless
        // we decide we want to.
        //
        // Note: Case (2) of the motivation shouldn't really apply
        // any more, because we end up having to support similar
        // inteference for built-in binary math functions where
        // vectors and scalars might be combined (and where defining
        // additional overloads to cover all the combinations doesn't
        // seem practical or desirable).
        //
        // TODO: We should consider whether dropping these extra
        // overloads is possible and worth it. The optimization
        // concern (1) could possibly be addressed in specific
        // back-ends. The issue (3) about not wanting to support
        // implicit scalar-to-matrix conversion may be moot if
        // we end up needing to support mixed scalar/matrix input
        // for builtin in non-operator functions anyway.

        // scalar-vector and scalar-matrix
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << leftType << " left, vector<" << rightType << ",N> right);\n";

        sb << "__generic<let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" << leftType << " left, matrix<" << rightType << ",N,M> right);\n";

        // vector-scalar and matrix-scalar
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, " << rightType << " right);\n";

        sb << "__generic<let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, " << rightType << " right);\n";
    }

    // Synthesize generic versions
    if(op.interface)
    {
        char const* leftType = "T";
        char const* rightType = leftType;
        char const* resultType = leftType;

        if (op.flags & BOOL_RESULT) resultType = "bool";
        // TODO: handle `SHIFT`

        // scalar version
        sb << "__generic<T : " << op.interface << ">\n";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << leftType << " left, " << rightType << " right);\n";

        // vector version
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, vector<" << rightType << ",N> right);\n";

        // matrix version
        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, matrix<" << rightType << ",N,M> right);\n";

        // scalar-vector and scalar-matrix
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << leftType << " left, vector<" << rightType << ",N> right);\n";

        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" <<  leftType << " left, matrix<" << rightType << ",N,M> right);\n";

        // vector-scalar and matrix-scalar
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, " << rightType << " right);\n";

        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "[OverloadRank(10)]";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, " << rightType << " right);\n";
    }
}

// We will declare the shift operations entirely as generics
// rather than try to handle all the pairings of left-hand
// and right-hand side types.
//
static const struct ShiftOpInfo
{
    char const* name;
    char const* funcName;
    int op;
} kShiftOps[] =
{
    { "<<", "shl", kIROp_Lsh },
    { ">>", "shr", kIROp_Rsh },
};
for(auto info : kShiftOps) {
}}}}

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType>
__intrinsic_op($(info.op))
L operator$(info.name)(L left, R right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType>
__intrinsic_op($(info.op))
L __$(info.funcName)(L left, R right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType>
[__unsafeForceInlineEarly]
L operator$(info.name)=(in out L left, R right)
{
    left = left $(info.name) right;
    return left;
}

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>
__intrinsic_op($(info.op))
vector<L,N> operator$(info.name)(vector<L,N> left, vector<R,N> right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>
[__unsafeForceInlineEarly]
vector<L,N> operator$(info.name)=(in out vector<L,N> left, vector<R,N> right)
{
    left = left $(info.name) right;
    return left;
}

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>
__intrinsic_op($(info.op))
matrix<L,N,M> operator$(info.name)(matrix<L,N,M> left, matrix<R,N,M> right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int, let Layout : int>
[__unsafeForceInlineEarly]
matrix<L, N, M> operator$(info.name)=(in out matrix<L, N, M, Layout> left, matrix<R, N, M> right)
{
    left = left $(info.name) right;
    return left;
}

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>
__intrinsic_op($(info.op))
vector<L,N> operator$(info.name)(L left, vector<R,N> right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>
__intrinsic_op($(info.op))
matrix<L,N,M> operator$(info.name)(L left, matrix<R,N,M> right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>
__intrinsic_op($(info.op))
vector<L,N> operator$(info.name)(vector<L,N> left, R right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>
[__unsafeForceInlineEarly]
vector<L, N> operator$(info.name)=(in out vector<L, N> left, R right)
{
    left = left $(info.name) right;
    return left;
}

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>
__intrinsic_op($(info.op))
matrix<L,N,M> operator$(info.name)(matrix<L,N,M> left, R right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int, let Layout : int>
[__unsafeForceInlineEarly]
matrix<L,N,M> operator$(info.name)=(in out matrix<L,N,M, Layout> left, R right)
{
    left = left $(info.name) right;
    return left;
}

${{{{
}

    static const struct CompoundBinaryOpInfo
    {
        char const* name;
        char const* interface;
    } kCompoundBinaryOps[] =
    {
        { "+",  "__BuiltinArithmeticType" },
        { "-",  "__BuiltinArithmeticType" },
        { "*",  "__BuiltinArithmeticType" },
        { "/",  "__BuiltinArithmeticType" },
        { "%",  "__BuiltinIntegerType" },
        { "%",  "__BuiltinFloatingPointType" },
        { "&",  "__BuiltinLogicalType" },
        { "|",  "__BuiltinLogicalType" },
        { "^",  "__BuiltinLogicalType" },
    };
    for( auto op : kCompoundBinaryOps )
    {
    }}}}

    __generic<T : $(op.interface)>
    [__unsafeForceInlineEarly]
    T operator$(op.name)=(in out T left, T right)
    {
        left = left $(op.name) right;
        return left;
    }

    __generic<T : $(op.interface), let N : int>
    [__unsafeForceInlineEarly]
    vector<T,N> operator$(op.name)=(in out vector<T,N> left, vector<T,N> right)
    {
        left = left $(op.name) right;
        return left;
    }

    __generic<T : $(op.interface), let N : int>
    [__unsafeForceInlineEarly]
    vector<T,N> operator$(op.name)=(in out vector<T,N> left, T right)
    {
        left = left $(op.name) right;
        return left;
    }

    __generic<T : $(op.interface), let R : int, let C : int, let Layout : int>
    [__unsafeForceInlineEarly]
    matrix<T,R,C> operator$(op.name)=(in out matrix<T,R,C,Layout> left, matrix<T,R,C> right)
    {
        left = left $(op.name) right;
        return left;
    }

    __generic<T : $(op.interface), let R : int, let C : int, let Layout : int>
    [__unsafeForceInlineEarly]
    matrix<T,R,C> operator$(op.name)=(in out matrix<T,R,C, Layout> left, T right)
    {
        left = left $(op.name) right;
        return left;
    }

    ${{{{
    }

}}}}

//@ public:


// Bit cast
__generic<T, U>
[__unsafeForceInlineEarly]
__intrinsic_op($(kIROp_BitCast))
T bit_cast(U value);

// Create Existential object
__generic<T, U>
[__unsafeForceInlineEarly]
__intrinsic_op($(kIROp_CreateExistentialObject))
T createDynamicObject(uint typeId, U value);

// Reinterpret
__generic<T, U>
[__unsafeForceInlineEarly]
__intrinsic_op($(kIROp_Reinterpret))
T reinterpret(U value);

// Use an otherwise unused value
//
// This can be used to silence the warning about returning before initializing
// an out paramter.
__generic<T>
[__readNone]
[ForceInline]
void unused(inout T){}

// Specialized function

/// Given a string returns an integer hash of that string.
__intrinsic_op($(kIROp_GetStringHash))
int getStringHash(String string);

/// Use will produce a syntax error in downstream compiler
/// Useful for testing diagnostics around compilation errors of downstream compiler
/// It 'returns' an int so can be used in expressions without the front end complaining.
__target_intrinsic(hlsl, " @ ")
__target_intrinsic(glsl, " @ ")
__target_intrinsic(cuda, " @ ")
__target_intrinsic(cpp, " @ ")
int __SyntaxError();

/// For downstream compilers that allow sizeof/alignof/offsetof
/// Can't be called in the C/C++ style. Need to use __size_of<some_type>() as opposed to sizeof(some_type).
__generic<T>
__target_intrinsic(cuda, "sizeof($G0)")
__target_intrinsic(cpp, "sizeof($G0)")
[__readNone]
int __sizeOf();

__generic<T>
__target_intrinsic(cuda, "sizeof($T0)")
__target_intrinsic(cpp, "sizeof($T0)")
[__readNone]
int __sizeOf(T v);

__generic<T>
__target_intrinsic(cuda, "SLANG_ALIGN_OF($G0)")
__target_intrinsic(cpp, "SLANG_ALIGN_OF($G0)")
[__readNone]
int __alignOf();

__generic<T>
__target_intrinsic(cuda, "SLANG_ALIGN_OF($T0)")
__target_intrinsic(cpp, "SLANG_ALIGN_OF($T0)")
[__readNone]
int __alignOf(T v);

// It would be nice to have offsetof equivalent, but it's not clear how that would work in terms of the Slang language.
// Here we allow calculating the offset of a field in bytes from an *instance* of the type.
__generic<T,F>
__target_intrinsic(cuda, "int(((char*)&($1)) - ((char*)&($0)))")
__target_intrinsic(cpp, "int(((char*)&($1)) - ((char*)&($0))")
[__readNone]
int __offsetOf(in T t, in F field);

/// Mark beginning of "interlocked" operations in a fragment shader.
__glsl_extension(GL_ARB_fragment_shader_interlock)
__glsl_version(420)
void beginInvocationInterlock()
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "beginInvocationInterlockARB";
    case spirv:
        spirv_asm {
            OpCapability FragmentShaderPixelInterlockEXT;
            OpExtension "SPV_EXT_fragment_shader_interlock";
            OpExecutionMode __entryPoint PixelInterlockOrderedEXT;
            OpBeginInvocationInterlockEXT;
        };
    }
}

/// Mark end of "interlocked" operations in a fragment shader.
__glsl_extension(GL_ARB_fragment_shader_interlock)
__glsl_version(420)
void endInvocationInterlock()
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "endInvocationInterlockARB";
    case spirv:
        spirv_asm {
            OpCapability FragmentShaderPixelInterlockEXT;
            OpExtension "SPV_EXT_fragment_shader_interlock";
            OpExecutionMode __entryPoint PixelInterlockOrderedEXT;
            OpEndInvocationInterlockEXT;
        };
    }
}

// Operators to apply to `enum` types

//@ hidden:

__generic<E : __EnumType>
__intrinsic_op($(kIROp_Eql))
bool operator==(E left, E right);

__generic<E : __EnumType>
__intrinsic_op($(kIROp_Neq))
bool operator!=(E left, E right);

//@ public:

// public interfaces for generic arithmetic types.

__generic<T : IComparable>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
bool operator<(T v0, T v1)
{
    return v0.lessThan(v1);
}
__generic<T : IComparable>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
bool operator>(T v0, T v1)
{
    return v1.lessThan(v0);
}
__generic<T : IComparable>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
bool operator ==(T v0, T v1)
{
    return v0.equals(v1);
}
__generic<T : IComparable>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
bool operator >=(T v0, T v1)
{
    return v1.lessThan(v1);
}
__generic<T : IComparable>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
bool operator <=(T v0, T v1)
{
    return v0.lessThanOrEquals(v1);
}
__generic<T : IComparable>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
bool operator !=(T v0, T v1)
{
    return !v0.equals(v1);
}

${{{{
const char* arithmeticInterfaces[] = {"IArithmetic", "IFloat"};
const char* attribs[] = {"", "[TreatAsDifferentiable]"};
for (Index i = 0; i < 2; i++) {
    const auto interfaceName = arithmeticInterfaces[i];
    const auto attrib = attribs[i];
    Index overloadRank = i - 3;
}}}}
$(attrib)
__generic<T : $(interfaceName)>
[__unsafeForceInlineEarly]
[OverloadRank($(overloadRank))]
T operator +(T v0, T v1)
{
    return v0.add(v1);
}

$(attrib)
__generic<T : $(interfaceName)>
[__unsafeForceInlineEarly]
[OverloadRank($(overloadRank))]
T operator -(T v0, T v1)
{
    return v0.sub(v1);
}

$(attrib)
__generic<T : $(interfaceName)>
[__unsafeForceInlineEarly]
[OverloadRank($(overloadRank))]
T operator *(T v0, T v1)
{
    return v0.mul(v1);
}

$(attrib)
__generic<T : $(interfaceName)>
[__unsafeForceInlineEarly]
[OverloadRank($(overloadRank))]
T operator /(T v0, T v1)
{
    return v0.div(v1);
}

$(attrib)
__generic<T : $(interfaceName)>
[__unsafeForceInlineEarly]
[OverloadRank($(overloadRank))]
T operator %(T v0, T v1)
{
    return v0.mod(v1);
}

$(attrib)
__generic<T : $(interfaceName)>
[__unsafeForceInlineEarly]
[OverloadRank($(overloadRank))]
__prefix T operator -(T v0)
{
    return v0.neg();
}

${{{{
    } // foreach ["IArithmetic", "IFloat"]
}}}}

__generic<T : ILogical>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
T operator &(T v0, T v1)
{
    return v0.bitAnd(v1);
}
__generic<T : ILogical>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
T operator &&(T v0, T v1)
{
    return v0.and(v1);
}
__generic<T : ILogical>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
T operator |(T v0, T v1)
{
    return v0.bitOr(v1);
}
__generic<T : ILogical>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
T operator ||(T v0, T v1)
{
    return v0.or(v1);
}
__generic<T : ILogical>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
T operator ^(T v0, T v1)
{
    return v0.bitXor(v1);
}
__generic<T : ILogical>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
__prefix T operator ~(T v0)
{
    return v0.bitNot();
}
__generic<T : ILogical>
[__unsafeForceInlineEarly]
[OverloadRank(-10)]
__prefix T operator !(T v0)
{
    return v0.not();
}

// IR level type traits.

__generic<T>
__intrinsic_op($(kIROp_undefined))
T __declVal();

__generic<T>
__intrinsic_op($(kIROp_DefaultConstruct))
T __default();

__generic<T, U>
__intrinsic_op($(kIROp_TypeEquals))
bool __type_equals_impl(T t, U u);

__generic<T, U>
[__unsafeForceInlineEarly]
bool __type_equals(T t, U u)
{
    return __type_equals_impl(__declVal<T>(), __declVal<U>());
}

__generic<T, U>
[__unsafeForceInlineEarly]
bool __type_equals()
{
    return __type_equals_impl(__declVal<T>(), __declVal<U>());
}

__generic<T>
__intrinsic_op($(kIROp_IsBool))
bool __isBool_impl(T t);

__generic<T>
[__unsafeForceInlineEarly]
bool __isBool()
{
    return __isBool_impl(__declVal<T>());
}

__generic<T>
__intrinsic_op($(kIROp_IsInt))
bool __isInt_impl(T t);

__generic<T>
[__unsafeForceInlineEarly]
bool __isInt()
{
    return __isInt_impl(__declVal<T>());
}

__generic<T>
__intrinsic_op($(kIROp_IsFloat))
bool __isFloat_impl(T t);

__generic<T>
__intrinsic_op($(kIROp_IsHalf))
bool __isHalf_impl(T t);

__generic<T>
[__unsafeForceInlineEarly]
bool __isFloat()
{
    return __isFloat_impl(__declVal<T>());
}

__generic<T>
[__unsafeForceInlineEarly]
bool __isHalf()
{
    return __isHalf_impl(__declVal<T>());
}

__generic<T>
__intrinsic_op($(kIROp_IsUnsignedInt))
bool __isUnsignedInt_impl(T t);

__generic<T>
[__unsafeForceInlineEarly]
bool __isUnsignedInt()
{
    return __isUnsignedInt_impl(__declVal<T>());
}

__generic<T>
__intrinsic_op($(kIROp_IsSignedInt))
bool __isSignedInt_impl(T t);

__generic<T>
[__unsafeForceInlineEarly]
bool __isSignedInt()
{
    return __isSignedInt_impl(__declVal<T>());
}

__generic<T>
__intrinsic_op($(kIROp_IsVector))
bool __isVector_impl(T t);

__generic<T>
[__unsafeForceInlineEarly]
bool __isVector()
{
    return __isVector_impl(__declVal<T>());
}

__generic<T>
__intrinsic_op($(kIROp_GetNaturalStride))
int __naturalStrideOf_impl(T v);

__generic<T>
[__unsafeForceInlineEarly]
int __naturalStrideOf()
{
    return __naturalStrideOf_impl(__declVal<T>());
}

__intrinsic_op($(kIROp_TreatAsDynamicUniform))
T asDynamicUniform<T>(T v);

// Binding Attributes

__attributeTarget(DeclBase)
attribute_syntax [vk_binding(binding: int, set: int = 0)]			: GLSLBindingAttribute;

__attributeTarget(DeclBase)
attribute_syntax [gl_binding(binding: int, set: int = 0)]			: GLSLBindingAttribute;


__attributeTarget(VarDeclBase)
attribute_syntax [vk_shader_record]			                        : ShaderRecordAttribute;
__attributeTarget(VarDeclBase)
attribute_syntax [shader_record]			                        : ShaderRecordAttribute;

__attributeTarget(DeclBase)
attribute_syntax [vk_push_constant]									: PushConstantAttribute;
__attributeTarget(DeclBase)
attribute_syntax [push_constant]									: PushConstantAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [vk_location(locaiton : int)] : GLSLLocationAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [vk_index(index : int)] : GLSLIndexAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [vk_spirv_instruction(op : int, set : String = "")]     : SPIRVInstructionOpAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [spv_target_env_1_3] : SPIRVTargetEnv13Attribute;

__attributeTarget(VarDeclBase)
attribute_syntax [disable_array_flattening] : DisableArrayFlatteningAttribute;

// Statement Attributes

__attributeTarget(LoopStmt)
attribute_syntax [unroll(count: int = 0)]   : UnrollAttribute;

__attributeTarget(LoopStmt)
attribute_syntax [ForceUnroll(count: int = 0)]   : ForceUnrollAttribute;

__attributeTarget(LoopStmt)
attribute_syntax [loop]                 : LoopAttribute;

__attributeTarget(LoopStmt)
attribute_syntax [fastopt]              : FastOptAttribute;

__attributeTarget(LoopStmt)
attribute_syntax [allow_uav_condition]  : AllowUAVConditionAttribute;

__attributeTarget(LoopStmt)
attribute_syntax [MaxIters(count)]      : MaxItersAttribute;

__attributeTarget(IfStmt)
attribute_syntax [flatten]              : FlattenAttribute;

__attributeTarget(IfStmt)
__attributeTarget(SwitchStmt)
attribute_syntax [branch]               : BranchAttribute;

__attributeTarget(SwitchStmt)
attribute_syntax [forcecase]            : ForceCaseAttribute;

__attributeTarget(SwitchStmt)
attribute_syntax [call]                 : CallAttribute;

// Entry-point Attributes

// All Stages
__attributeTarget(FuncDecl)
attribute_syntax [shader(stage)]    : EntryPointAttribute;

// Hull Shader
__attributeTarget(FuncDecl)
attribute_syntax [maxtessfactor(factor: float)]     : MaxTessFactorAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [outputcontrolpoints(count: int)]  : OutputControlPointsAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [outputtopology(topology)]         : OutputTopologyAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [partitioning(mode)]               : PartitioningAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [patchconstantfunc(name)]          : PatchConstantFuncAttribute;

// Hull/Domain Shader
__attributeTarget(FuncDecl)
attribute_syntax [domain(domain)]   : DomainAttribute;

// Geometry Shader
__attributeTarget(FuncDecl)
attribute_syntax [maxvertexcount(count: int)]   : MaxVertexCountAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [instance(count: int)]         : InstanceAttribute;

// Fragment ("Pixel") Shader
__attributeTarget(FuncDecl)
attribute_syntax [earlydepthstencil]    : EarlyDepthStencilAttribute;

// Compute Shader
__attributeTarget(FuncDecl)
attribute_syntax [numthreads(x: int, y: int = 1, z: int = 1)]   : NumThreadsAttribute;

//
__attributeTarget(VarDeclBase)
attribute_syntax [__vulkanRayPayload(location : int = -1)] : VulkanRayPayloadAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [__vulkanCallablePayload(location : int = -1)] : VulkanCallablePayloadAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [__vulkanHitObjectAttributes(location : int = -1)] : VulkanHitObjectAttributesAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [__vulkanHitAttributes] : VulkanHitAttributesAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [mutating] : MutatingAttribute;

__attributeTarget(SetterDecl)
attribute_syntax [nonmutating] : NonmutatingAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [constref] : ConstRefAttribute;

    /// Indicates that a function computes its result as a function of its arguments without loading/storing any memory or other state.
    ///
    /// This is equivalent to the LLVM `readnone` function attribute.
__attributeTarget(FunctionDeclBase)
attribute_syntax [__readNone] : ReadNoneAttribute;

enum _AttributeTargets
{
    Struct = $( (int) UserDefinedAttributeTargets::Struct),
    Var = $( (int) UserDefinedAttributeTargets::Var),
    Function = $( (int) UserDefinedAttributeTargets::Function),
};
__attributeTarget(StructDecl)
attribute_syntax [__AttributeUsage(target : _AttributeTargets)] : AttributeUsageAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [format(format : String)] : FormatAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [vk_image_format(format : String)] : FormatAttribute;

__attributeTarget(Decl)
attribute_syntax [allow(diagnostic: String)] : AllowAttribute;

__attributeTarget(Decl)
attribute_syntax[require(capability)] : RequireCapabilityAttribute;

// Linking
__attributeTarget(Decl)
attribute_syntax [__extern] : ExternAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [__unsafeForceInlineEarly] : UnsafeForceInlineEarlyAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [ForceInline] : ForceInlineAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [OverloadRank] : OverloadRankAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [DllImport(modulePath: String)] : DllImportAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [DllExport] : DllExportAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [TorchEntryPoint] : TorchEntryPointAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [CudaDeviceExport] : CudaDeviceExportAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [CudaHost] : CudaHostAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [CudaKernel] : CudaKernelAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [CUDADeviceExport] : CudaDeviceExportAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [CUDAHost] : CudaHostAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [CUDAKernel] : CudaKernelAttribute;

__attributeTarget(FuncDecl)
attribute_syntax[AutoPyBindCUDA] : AutoPyBindCudaAttribute;

__attributeTarget(AggTypeDecl)
attribute_syntax [PyExport(name: String)] : PyExportAttribute;

__attributeTarget(InterfaceDecl)
attribute_syntax [COM(guid: String)] : ComInterfaceAttribute;

// Inheritance Control
__attributeTarget(AggTypeDecl)
attribute_syntax [sealed] : SealedAttribute;

__attributeTarget(AggTypeDecl)
attribute_syntax [open] : OpenAttribute;

__attributeTarget(InterfaceDecl)
attribute_syntax [anyValueSize(size:int)] : AnyValueSizeAttribute;

__attributeTarget(InterfaceDecl)
attribute_syntax [Specialize] : SpecializeAttribute;

__attributeTarget(DeclBase)
attribute_syntax [builtin] : BuiltinAttribute;

__attributeTarget(DeclBase)
attribute_syntax [__requiresNVAPI] : RequiresNVAPIAttribute;

__attributeTarget(DeclBase)
attribute_syntax [__AlwaysFoldIntoUseSiteAttribute] : AlwaysFoldIntoUseSiteAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [noinline] : NoInlineAttribute;

__attributeTarget(StructDecl)
attribute_syntax [payload] : PayloadAttribute;

__attributeTarget(DeclBase)
attribute_syntax [deprecated(message: String)] : DeprecatedAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [PreferRecompute] : PreferRecomputeAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [PreferCheckpoint] : PreferCheckpointAttribute;

__attributeTarget(DeclBase)
attribute_syntax [KnownBuiltin(name : String)] : KnownBuiltinAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [NonUniformReturn] : NonDynamicUniformAttribute;
