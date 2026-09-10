implementing numerics;

// These conformances implement the public numeric requirements in terms of the established
// core-module operations. Calls use the distinguished builtin-adapter names, or an explicitly
// shaped builtin overload, so they cannot resolve back through a public interface-generic wrapper
// to the witness currently being implemented and form a dispatch cycle.

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension half : IScalarFloatingPoint
{
    [__unsafeForceInlineEarly]
    This floor()
    {
        return __numericInterfaceBuiltinFloor(this);
    }

    [__unsafeForceInlineEarly]
    This ceiling()
    {
        return __numericInterfaceBuiltinCeil(this);
    }

    [__unsafeForceInlineEarly]
    This rounded()
    {
        return __numericInterfaceBuiltinRound(this);
    }

    [__unsafeForceInlineEarly]
    [require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
    This roundedToNearestEven()
    {
        return __numericInterfaceBuiltinRoundEven(this);
    }

    [__unsafeForceInlineEarly]
    This truncated()
    {
        return __numericInterfaceBuiltinTruncate(this);
    }

    [__unsafeForceInlineEarly]
    This fractionalPart()
    {
        return __numericInterfaceBuiltinFractionalPart(this);
    }

    [__unsafeForceInlineEarly]
    This remainderWhenDividedBy(This divisor)
    {
        return __numericInterfaceBuiltinRemainder(this, divisor);
    }

    [__unsafeForceInlineEarly]
    This copyingSignFrom(This signSource)
    {
        return __numericInterfaceBuiltinCopySign(this, signSource);
    }

    [__unsafeForceInlineEarly]
    This fractionalPart(out This integralPart)
    {
        return __numericInterfaceBuiltinSplitFraction(this, integralPart);
    }

    [__unsafeForceInlineEarly]
    bool isNaN()
    {
        return __numericInterfaceBuiltinIsNaN(this);
    }

    [__unsafeForceInlineEarly]
    bool isFinite()
    {
        return __numericInterfaceBuiltinIsFinite(this);
    }

    [__unsafeForceInlineEarly]
    bool isInfinite()
    {
        return __numericInterfaceBuiltinIsInfinite(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension half : ITrigonometricFunctions
{
    [__unsafeForceInlineEarly]
    This sin()
    {
        return __numericInterfaceBuiltinSin(this);
    }

    [__unsafeForceInlineEarly]
    This cos()
    {
        return __numericInterfaceBuiltinCos(this);
    }

    [__unsafeForceInlineEarly]
    This tan()
    {
        return __numericInterfaceBuiltinTan(this);
    }

    [__unsafeForceInlineEarly]
    void sincos(out This sineResult, out This cosineResult)
    {
        __numericInterfaceBuiltinSinCos(this, sineResult, cosineResult);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension half : IExponentialFunctions
{
    [__unsafeForceInlineEarly]
    This exponential()
    {
        return __numericInterfaceBuiltinExp(this);
    }

    [__unsafeForceInlineEarly]
    This exponentialWithBase2()
    {
        return __numericInterfaceBuiltinExp2(this);
    }

    [__unsafeForceInlineEarly]
    This exponentialWithBase10()
    {
        return __numericInterfaceBuiltinExp10(this);
    }

    [__unsafeForceInlineEarly]
    This logarithm()
    {
        return __numericInterfaceBuiltinLog(this);
    }

    [__unsafeForceInlineEarly]
    This logarithmWithBase2()
    {
        return __numericInterfaceBuiltinLog2(this);
    }

    [__unsafeForceInlineEarly]
    This logarithmWithBase10()
    {
        return __numericInterfaceBuiltinLog10(this);
    }

    [__unsafeForceInlineEarly]
    This raisedToThePowerOf(This exponent)
    {
        return __numericInterfaceBuiltinPow(this, exponent);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension half : IInverseTrigonometricFunctions
{
    [__unsafeForceInlineEarly]
    This asin()
    {
        return __numericInterfaceBuiltinAsin(this);
    }

    [__unsafeForceInlineEarly]
    This acos()
    {
        return __numericInterfaceBuiltinAcos(this);
    }

    [__unsafeForceInlineEarly]
    This atan()
    {
        return __numericInterfaceBuiltinAtan(this);
    }

    [__unsafeForceInlineEarly]
    This atan2(This denominator)
    {
        return __numericInterfaceBuiltinAtan2(this, denominator);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension half : IHyperbolicFunctions
{
    [__unsafeForceInlineEarly]
    This sinh()
    {
        return __numericInterfaceBuiltinSinh(this);
    }

    [__unsafeForceInlineEarly]
    This cosh()
    {
        return __numericInterfaceBuiltinCosh(this);
    }

    [__unsafeForceInlineEarly]
    This tanh()
    {
        return __numericInterfaceBuiltinTanh(this);
    }

    [__unsafeForceInlineEarly]
    This asinh()
    {
        return __numericInterfaceBuiltinAsinh(this);
    }

    [__unsafeForceInlineEarly]
    This acosh()
    {
        return __numericInterfaceBuiltinAcosh(this);
    }

    [__unsafeForceInlineEarly]
    This atanh()
    {
        return __numericInterfaceBuiltinAtanh(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension half : IRootFunctions
{
    [__unsafeForceInlineEarly]
    This squareRoot()
    {
        return __numericInterfaceBuiltinSqrt(this);
    }

    [__unsafeForceInlineEarly]
    This reciprocalSquareRoot()
    {
        return __numericInterfaceBuiltinReciprocalSqrt(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension half : IRealOrderingFunctions
{
    [__unsafeForceInlineEarly]
    This minimum(This other)
    {
        return __numericsBuiltinMinimum(this, other);
    }

    [__unsafeForceInlineEarly]
    This maximum(This other)
    {
        return __numericsBuiltinMaximum(this, other);
    }

    [__unsafeForceInlineEarly]
    This step(This edge)
    {
        return this < edge ? This.zero() : This.one();
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension float : IScalarFloatingPoint
{
    [__unsafeForceInlineEarly]
    This floor()
    {
        return __numericInterfaceBuiltinFloor(this);
    }

    [__unsafeForceInlineEarly]
    This ceiling()
    {
        return __numericInterfaceBuiltinCeil(this);
    }

    [__unsafeForceInlineEarly]
    This rounded()
    {
        return __numericInterfaceBuiltinRound(this);
    }

    [__unsafeForceInlineEarly]
    [require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
    This roundedToNearestEven()
    {
        return __numericInterfaceBuiltinRoundEven(this);
    }

    [__unsafeForceInlineEarly]
    This truncated()
    {
        return __numericInterfaceBuiltinTruncate(this);
    }

    [__unsafeForceInlineEarly]
    This fractionalPart()
    {
        return __numericInterfaceBuiltinFractionalPart(this);
    }

    [__unsafeForceInlineEarly]
    This remainderWhenDividedBy(This divisor)
    {
        return __numericInterfaceBuiltinRemainder(this, divisor);
    }

    [__unsafeForceInlineEarly]
    This copyingSignFrom(This signSource)
    {
        return __numericInterfaceBuiltinCopySign(this, signSource);
    }

    [__unsafeForceInlineEarly]
    This fractionalPart(out This integralPart)
    {
        return __numericInterfaceBuiltinSplitFraction(this, integralPart);
    }

    [__unsafeForceInlineEarly]
    bool isNaN()
    {
        return __numericInterfaceBuiltinIsNaN(this);
    }

    [__unsafeForceInlineEarly]
    bool isFinite()
    {
        return __numericInterfaceBuiltinIsFinite(this);
    }

    [__unsafeForceInlineEarly]
    bool isInfinite()
    {
        return __numericInterfaceBuiltinIsInfinite(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension float : ITrigonometricFunctions
{
    [__unsafeForceInlineEarly]
    This sin()
    {
        return __numericInterfaceBuiltinSin(this);
    }

    [__unsafeForceInlineEarly]
    This cos()
    {
        return __numericInterfaceBuiltinCos(this);
    }

    [__unsafeForceInlineEarly]
    This tan()
    {
        return __numericInterfaceBuiltinTan(this);
    }

    [__unsafeForceInlineEarly]
    void sincos(out This sineResult, out This cosineResult)
    {
        __numericInterfaceBuiltinSinCos(this, sineResult, cosineResult);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension float : IExponentialFunctions
{
    [__unsafeForceInlineEarly]
    This exponential()
    {
        return __numericInterfaceBuiltinExp(this);
    }

    [__unsafeForceInlineEarly]
    This exponentialWithBase2()
    {
        return __numericInterfaceBuiltinExp2(this);
    }

    [__unsafeForceInlineEarly]
    This exponentialWithBase10()
    {
        return __numericInterfaceBuiltinExp10(this);
    }

    [__unsafeForceInlineEarly]
    This logarithm()
    {
        return __numericInterfaceBuiltinLog(this);
    }

    [__unsafeForceInlineEarly]
    This logarithmWithBase2()
    {
        return __numericInterfaceBuiltinLog2(this);
    }

    [__unsafeForceInlineEarly]
    This logarithmWithBase10()
    {
        return __numericInterfaceBuiltinLog10(this);
    }

    [__unsafeForceInlineEarly]
    This raisedToThePowerOf(This exponent)
    {
        return __numericInterfaceBuiltinPow(this, exponent);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension float : IInverseTrigonometricFunctions
{
    [__unsafeForceInlineEarly]
    This asin()
    {
        return __numericInterfaceBuiltinAsin(this);
    }

    [__unsafeForceInlineEarly]
    This acos()
    {
        return __numericInterfaceBuiltinAcos(this);
    }

    [__unsafeForceInlineEarly]
    This atan()
    {
        return __numericInterfaceBuiltinAtan(this);
    }

    [__unsafeForceInlineEarly]
    This atan2(This denominator)
    {
        return __numericInterfaceBuiltinAtan2(this, denominator);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension float : IHyperbolicFunctions
{
    [__unsafeForceInlineEarly]
    This sinh()
    {
        return __numericInterfaceBuiltinSinh(this);
    }

    [__unsafeForceInlineEarly]
    This cosh()
    {
        return __numericInterfaceBuiltinCosh(this);
    }

    [__unsafeForceInlineEarly]
    This tanh()
    {
        return __numericInterfaceBuiltinTanh(this);
    }

    [__unsafeForceInlineEarly]
    This asinh()
    {
        return __numericInterfaceBuiltinAsinh(this);
    }

    [__unsafeForceInlineEarly]
    This acosh()
    {
        return __numericInterfaceBuiltinAcosh(this);
    }

    [__unsafeForceInlineEarly]
    This atanh()
    {
        return __numericInterfaceBuiltinAtanh(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension float : IRootFunctions
{
    [__unsafeForceInlineEarly]
    This squareRoot()
    {
        return __numericInterfaceBuiltinSqrt(this);
    }

    [__unsafeForceInlineEarly]
    This reciprocalSquareRoot()
    {
        return __numericInterfaceBuiltinReciprocalSqrt(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension float : IRealOrderingFunctions
{
    [__unsafeForceInlineEarly]
    This minimum(This other)
    {
        return __numericsBuiltinMinimum(this, other);
    }

    [__unsafeForceInlineEarly]
    This maximum(This other)
    {
        return __numericsBuiltinMaximum(this, other);
    }

    [__unsafeForceInlineEarly]
    This step(This edge)
    {
        return this < edge ? This.zero() : This.one();
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension double : IScalarFloatingPoint
{
    [__unsafeForceInlineEarly]
    This floor()
    {
        return __numericInterfaceBuiltinFloor(this);
    }

    [__unsafeForceInlineEarly]
    This ceiling()
    {
        return __numericInterfaceBuiltinCeil(this);
    }

    [__unsafeForceInlineEarly]
    This rounded()
    {
        return __numericInterfaceBuiltinRound(this);
    }

    [__unsafeForceInlineEarly]
    [require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
    This roundedToNearestEven()
    {
        return __numericInterfaceBuiltinRoundEven(this);
    }

    [__unsafeForceInlineEarly]
    This truncated()
    {
        return __numericInterfaceBuiltinTruncate(this);
    }

    [__unsafeForceInlineEarly]
    This fractionalPart()
    {
        return __numericInterfaceBuiltinFractionalPart(this);
    }

    [__unsafeForceInlineEarly]
    This remainderWhenDividedBy(This divisor)
    {
        return __numericInterfaceBuiltinRemainder(this, divisor);
    }

    [__unsafeForceInlineEarly]
    This copyingSignFrom(This signSource)
    {
        return __numericInterfaceBuiltinCopySign(this, signSource);
    }

    [__unsafeForceInlineEarly]
    This fractionalPart(out This integralPart)
    {
        return __numericInterfaceBuiltinSplitFraction(this, integralPart);
    }

    [__unsafeForceInlineEarly]
    bool isNaN()
    {
        return __numericInterfaceBuiltinIsNaN(this);
    }

    [__unsafeForceInlineEarly]
    bool isFinite()
    {
        return __numericInterfaceBuiltinIsFinite(this);
    }

    [__unsafeForceInlineEarly]
    bool isInfinite()
    {
        return __numericInterfaceBuiltinIsInfinite(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension double : ITrigonometricFunctions
{
    [__unsafeForceInlineEarly]
    This sin()
    {
        return __numericInterfaceBuiltinSin(this);
    }

    [__unsafeForceInlineEarly]
    This cos()
    {
        return __numericInterfaceBuiltinCos(this);
    }

    [__unsafeForceInlineEarly]
    This tan()
    {
        return __numericInterfaceBuiltinTan(this);
    }

    [__unsafeForceInlineEarly]
    void sincos(out This sineResult, out This cosineResult)
    {
        __numericInterfaceBuiltinSinCos(this, sineResult, cosineResult);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension double : IExponentialFunctions
{
    [__unsafeForceInlineEarly]
    This exponential()
    {
        return __numericInterfaceBuiltinExp(this);
    }

    [__unsafeForceInlineEarly]
    This exponentialWithBase2()
    {
        return __numericInterfaceBuiltinExp2(this);
    }

    [__unsafeForceInlineEarly]
    This exponentialWithBase10()
    {
        return __numericInterfaceBuiltinExp10(this);
    }

    [__unsafeForceInlineEarly]
    This logarithm()
    {
        return __numericInterfaceBuiltinLog(this);
    }

    [__unsafeForceInlineEarly]
    This logarithmWithBase2()
    {
        return __numericInterfaceBuiltinLog2(this);
    }

    [__unsafeForceInlineEarly]
    This logarithmWithBase10()
    {
        return __numericInterfaceBuiltinLog10(this);
    }

    [__unsafeForceInlineEarly]
    This raisedToThePowerOf(This exponent)
    {
        return __numericInterfaceBuiltinPow(this, exponent);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension double : IInverseTrigonometricFunctions
{
    [__unsafeForceInlineEarly]
    This asin()
    {
        return __numericInterfaceBuiltinAsin(this);
    }

    [__unsafeForceInlineEarly]
    This acos()
    {
        return __numericInterfaceBuiltinAcos(this);
    }

    [__unsafeForceInlineEarly]
    This atan()
    {
        return __numericInterfaceBuiltinAtan(this);
    }

    [__unsafeForceInlineEarly]
    This atan2(This denominator)
    {
        return __numericInterfaceBuiltinAtan2(this, denominator);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension double : IHyperbolicFunctions
{
    [__unsafeForceInlineEarly]
    This sinh()
    {
        return __numericInterfaceBuiltinSinh(this);
    }

    [__unsafeForceInlineEarly]
    This cosh()
    {
        return __numericInterfaceBuiltinCosh(this);
    }

    [__unsafeForceInlineEarly]
    This tanh()
    {
        return __numericInterfaceBuiltinTanh(this);
    }

    [__unsafeForceInlineEarly]
    This asinh()
    {
        return __numericInterfaceBuiltinAsinh(this);
    }

    [__unsafeForceInlineEarly]
    This acosh()
    {
        return __numericInterfaceBuiltinAcosh(this);
    }

    [__unsafeForceInlineEarly]
    This atanh()
    {
        return __numericInterfaceBuiltinAtanh(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension double : IRootFunctions
{
    [__unsafeForceInlineEarly]
    This squareRoot()
    {
        return __numericInterfaceBuiltinSqrt(this);
    }

    [__unsafeForceInlineEarly]
    This reciprocalSquareRoot()
    {
        return __numericInterfaceBuiltinReciprocalSqrt(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension double : IRealOrderingFunctions
{
    [__unsafeForceInlineEarly]
    This minimum(This other)
    {
        return __numericsBuiltinMinimum(this, other);
    }

    [__unsafeForceInlineEarly]
    This maximum(This other)
    {
        return __numericsBuiltinMaximum(this, other);
    }

    [__unsafeForceInlineEarly]
    This step(This edge)
    {
        return this < edge ? This.zero() : This.one();
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IScalarFloatingPoint,
    let N : int> vector<T, N> : IFloatingPoint
{
    [__unsafeForceInlineEarly]
    This floor()
    {
        return __numericInterfaceBuiltinFloor(this);
    }

    [__unsafeForceInlineEarly]
    This ceiling()
    {
        return __numericInterfaceBuiltinCeil(this);
    }

    [__unsafeForceInlineEarly]
    This rounded()
    {
        return __numericInterfaceBuiltinRound(this);
    }

    [__unsafeForceInlineEarly]
    [require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
    This roundedToNearestEven()
    {
        return __numericInterfaceBuiltinRoundEven(this);
    }

    [__unsafeForceInlineEarly]
    This truncated()
    {
        return __numericInterfaceBuiltinTruncate(this);
    }

    [__unsafeForceInlineEarly]
    This fractionalPart()
    {
        return __numericInterfaceBuiltinFractionalPart(this);
    }

    [__unsafeForceInlineEarly]
    This remainderWhenDividedBy(This divisor)
    {
        return __numericInterfaceBuiltinRemainder(this, divisor);
    }

    [__unsafeForceInlineEarly]
    This copyingSignFrom(This signSource)
    {
        return __numericInterfaceBuiltinCopySign(this, signSource);
    }

    [__unsafeForceInlineEarly]
    This fractionalPart(out This integralPart)
    {
        return __numericInterfaceBuiltinSplitFraction(this, integralPart);
    }

    [__unsafeForceInlineEarly]
    Mask isNaN()
    {
        return __numericInterfaceBuiltinIsNaN(this);
    }

    [__unsafeForceInlineEarly]
    Mask isFinite()
    {
        return __numericInterfaceBuiltinIsFinite(this);
    }

    [__unsafeForceInlineEarly]
    Mask isInfinite()
    {
        return __numericInterfaceBuiltinIsInfinite(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & ITrigonometricFunctions,
    let N : int> vector<T, N> : ITrigonometricFunctions
{
    [__unsafeForceInlineEarly]
    This sin()
    {
        return __numericInterfaceBuiltinSin(this);
    }

    [__unsafeForceInlineEarly]
    This cos()
    {
        return __numericInterfaceBuiltinCos(this);
    }

    [__unsafeForceInlineEarly]
    This tan()
    {
        return __numericInterfaceBuiltinTan(this);
    }

    [__unsafeForceInlineEarly]
    void sincos(out This sineResult, out This cosineResult)
    {
        __numericInterfaceBuiltinSinCos(this, sineResult, cosineResult);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IExponentialFunctions,
    let N : int> vector<T, N> : IExponentialFunctions
{
    [__unsafeForceInlineEarly]
    This exponential()
    {
        return __numericInterfaceBuiltinExp(this);
    }

    [__unsafeForceInlineEarly]
    This exponentialWithBase2()
    {
        return __numericInterfaceBuiltinExp2(this);
    }

    [__unsafeForceInlineEarly]
    This exponentialWithBase10()
    {
        return __numericInterfaceBuiltinExp10(this);
    }

    [__unsafeForceInlineEarly]
    This logarithm()
    {
        return __numericInterfaceBuiltinLog(this);
    }

    [__unsafeForceInlineEarly]
    This logarithmWithBase2()
    {
        return __numericInterfaceBuiltinLog2(this);
    }

    [__unsafeForceInlineEarly]
    This logarithmWithBase10()
    {
        return __numericInterfaceBuiltinLog10(this);
    }

    [__unsafeForceInlineEarly]
    This raisedToThePowerOf(This exponent)
    {
        return __numericInterfaceBuiltinPow(this, exponent);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IInverseTrigonometricFunctions,
    let N : int> vector<T, N> : IInverseTrigonometricFunctions
{
    [__unsafeForceInlineEarly]
    This asin()
    {
        return __numericInterfaceBuiltinAsin(this);
    }

    [__unsafeForceInlineEarly]
    This acos()
    {
        return __numericInterfaceBuiltinAcos(this);
    }

    [__unsafeForceInlineEarly]
    This atan()
    {
        return __numericInterfaceBuiltinAtan(this);
    }

    [__unsafeForceInlineEarly]
    This atan2(This denominator)
    {
        return __numericInterfaceBuiltinAtan2(this, denominator);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IHyperbolicFunctions,
    let N : int> vector<T, N> : IHyperbolicFunctions
{
    [__unsafeForceInlineEarly]
    This sinh()
    {
        return __numericInterfaceBuiltinSinh(this);
    }

    [__unsafeForceInlineEarly]
    This cosh()
    {
        return __numericInterfaceBuiltinCosh(this);
    }

    [__unsafeForceInlineEarly]
    This tanh()
    {
        return __numericInterfaceBuiltinTanh(this);
    }

    [__unsafeForceInlineEarly]
    This asinh()
    {
        return __numericInterfaceBuiltinAsinh(this);
    }

    [__unsafeForceInlineEarly]
    This acosh()
    {
        return __numericInterfaceBuiltinAcosh(this);
    }

    [__unsafeForceInlineEarly]
    This atanh()
    {
        return __numericInterfaceBuiltinAtanh(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IRootFunctions,
    let N : int> vector<T, N> : IRootFunctions
{
    [__unsafeForceInlineEarly]
    This squareRoot()
    {
        return __numericInterfaceBuiltinSqrt(this);
    }

    [__unsafeForceInlineEarly]
    This reciprocalSquareRoot()
    {
        return __numericInterfaceBuiltinReciprocalSqrt(this);
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IScalarReal,
    let N : int> vector<T, N> : IRealOrderingFunctions
{
    [__unsafeForceInlineEarly]
    This minimum(This other)
    {
        return __numericsBuiltinMinimum(this, other);
    }

    [__unsafeForceInlineEarly]
    This maximum(This other)
    {
        return __numericsBuiltinMaximum(this, other);
    }

    [__unsafeForceInlineEarly]
    This step(This edge)
    {
        This result;
        [ForceUnroll]
        for (int i = 0; i < N; ++i)
            result[i] = this[i] < edge[i] ? T.zero() : T.one();
        return result;
    }
}

// Existing matrix math overloads omit the layout parameter from their signatures and therefore
// return a default-layout matrix. Implement these witnesses component-wise so a generic T -> T
// operation preserves the caller's matrix layout.
[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IScalarFloatingPoint,
    let R : int,
    let C : int,
    let L : int> matrix<T, R, C, L> : IFloatingPoint
{
    [__unsafeForceInlineEarly]
    This floor()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinFloor(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This ceiling()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinCeil(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This rounded()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinRound(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    [require(cpp_cuda_glsl_hlsl_metal_spirv_llvm, sm_4_0_version)]
    This roundedToNearestEven()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinRoundEven(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This truncated()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinTruncate(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This fractionalPart()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] =
                    __numericInterfaceBuiltinFractionalPart(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This remainderWhenDividedBy(This divisor)
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinRemainder(
                    this[row][column], divisor[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This copyingSignFrom(This signSource)
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinCopySign(
                    this[row][column], signSource[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This fractionalPart(out This integralPart)
    {
        This fractionalPart;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                fractionalPart[row][column] = __numericInterfaceBuiltinSplitFraction(
                    this[row][column], integralPart[row][column]);
        }
        return fractionalPart;
    }

    [__unsafeForceInlineEarly]
    Mask isNaN()
    {
        Mask result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinIsNaN(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    Mask isFinite()
    {
        Mask result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinIsFinite(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    Mask isInfinite()
    {
        Mask result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinIsInfinite(this[row][column]);
        }
        return result;
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & ITrigonometricFunctions,
    let R : int,
    let C : int,
    let L : int> matrix<T, R, C, L> : ITrigonometricFunctions
{
    [__unsafeForceInlineEarly]
    This sin()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinSin(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This cos()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinCos(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This tan()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinTan(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    void sincos(out This sineResult, out This cosineResult)
    {
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
            {
                T sineElement;
                T cosineElement;
                __numericInterfaceBuiltinSinCos(
                    this[row][column],
                    sineElement,
                    cosineElement);
                sineResult[row][column] = sineElement;
                cosineResult[row][column] = cosineElement;
            }
        }
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IExponentialFunctions,
    let R : int,
    let C : int,
    let L : int> matrix<T, R, C, L> : IExponentialFunctions
{
    [__unsafeForceInlineEarly]
    This exponential()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinExp(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This exponentialWithBase2()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinExp2(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This exponentialWithBase10()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinExp10(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This logarithm()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinLog(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This logarithmWithBase2()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinLog2(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This logarithmWithBase10()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinLog10(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This raisedToThePowerOf(This exponent)
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
            {
                result[row][column] = __numericInterfaceBuiltinPow(
                    this[row][column],
                    exponent[row][column]);
            }
        }
        return result;
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IInverseTrigonometricFunctions,
    let R : int,
    let C : int,
    let L : int> matrix<T, R, C, L> : IInverseTrigonometricFunctions
{
    [__unsafeForceInlineEarly]
    This asin()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinAsin(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This acos()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinAcos(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This atan()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinAtan(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This atan2(This denominator)
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
            {
                result[row][column] = __numericInterfaceBuiltinAtan2(
                    this[row][column],
                    denominator[row][column]);
            }
        }
        return result;
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IHyperbolicFunctions,
    let R : int,
    let C : int,
    let L : int> matrix<T, R, C, L> : IHyperbolicFunctions
{
    [__unsafeForceInlineEarly]
    This sinh()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinSinh(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This cosh()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinCosh(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This tanh()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinTanh(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This asinh()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinAsinh(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This acosh()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinAcosh(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This atanh()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinAtanh(this[row][column]);
        }
        return result;
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IRootFunctions,
    let R : int,
    let C : int,
    let L : int> matrix<T, R, C, L> : IRootFunctions
{
    [__unsafeForceInlineEarly]
    This squareRoot()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] = __numericInterfaceBuiltinSqrt(this[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This reciprocalSquareRoot()
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] =
                    __numericInterfaceBuiltinReciprocalSqrt(this[row][column]);
        }
        return result;
    }
}

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm, sm_4_0_version)]
public extension<
    T : __BuiltinFloatingPointType & IScalarReal,
    let R : int,
    let C : int,
    let L : int> matrix<T, R, C, L> : IRealOrderingFunctions
{
    [__unsafeForceInlineEarly]
    This minimum(This other)
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] =
                    __numericsBuiltinMinimum(this[row][column], other[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This maximum(This other)
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
                result[row][column] =
                    __numericsBuiltinMaximum(this[row][column], other[row][column]);
        }
        return result;
    }

    [__unsafeForceInlineEarly]
    This step(This edge)
    {
        This result;
        [ForceUnroll]
        for (int row = 0; row < R; ++row)
        {
            [ForceUnroll]
            for (int column = 0; column < C; ++column)
            {
                result[row][column] =
                    this[row][column] < edge[row][column] ? T.zero() : T.one();
            }
        }
        return result;
    }
}
