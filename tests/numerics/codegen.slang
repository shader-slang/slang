// Verify that numeric-interface witnesses specialize to concrete target types and operations
// without retaining interface or witness artifacts.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=EXTREMA-HLSL): -target hlsl -entry extremaMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=EXTREMA_HLSL_NO_ARTIFACT): -target hlsl -entry extremaMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=EXTREMA-SPIRV): -target spirv-asm -entry extremaMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=EXTREMA_SPIRV_NO_ARTIFACT): -target spirv-asm -entry extremaMain -stage compute -experimental-feature
//TEST:SIMPLE: -target metal -entry computeMain -stage compute -o numeric-interfaces-codegen.metal -experimental-feature
//TEST:SIMPLE: -target cuda -entry computeMain -stage compute -o numeric-interfaces-codegen.cuda.cpp -experimental-feature
//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -o numeric-interfaces-codegen.cpp -experimental-feature
//TEST:SIMPLE: -target wgsl -entry computeMain -stage compute -o numeric-interfaces-codegen.wgsl -experimental-feature

// HLSL-NOT: __numericInterface
// HLSL-NOT: IElementaryFunctions
// HLSL-NOT: IReal
// HLSL: dot(
// HLSL-NOT: __numericsBuiltinDot
// HLSL-NOT: IDotProduct
// HLSL-NOT: __numericInterface
// HLSL-NOT: IElementaryFunctions
// HLSL-NOT: IReal
// HLSL: sin(
// HLSL-NOT: __numericInterface
// HLSL-NOT: IElementaryFunctions
// HLSL-NOT: IReal
// HLSL: exp(
// HLSL-NOT: __numericInterface
// HLSL-NOT: IElementaryFunctions
// HLSL-NOT: IReal

// SPIRV-NOT: __numericInterface
// SPIRV-NOT: IElementaryFunctions
// SPIRV-NOT: IReal

// Extrema calls made through the interface must specialize to native target operations. In
// particular, no witness or numerics-module adapter should survive specialization.
// EXTREMA-HLSL-COUNT-6: return {{(min|max)}}(

// Keep whole-output negative assertions in dedicated prefixes: when a negative directive shares a
// prefix with positive checks, FileCheck only applies it to the region before the next match.
// EXTREMA_HLSL_NO_ARTIFACT-NOT: INumericExtrema
// EXTREMA_HLSL_NO_ARTIFACT-NOT: __numericsBuiltin

// EXTREMA-SPIRV-DAG: %[[V3_INT:[A-Za-z0-9_]+]] = OpTypeVector %int 3
// EXTREMA-SPIRV-DAG: %[[V2_UINT:[A-Za-z0-9_]+]] = OpTypeVector %uint 2
// EXTREMA-SPIRV-DAG: OpExtInst %float {{.*}} FMin
// EXTREMA-SPIRV-DAG: OpExtInst %float {{.*}} FMax
// EXTREMA-SPIRV-DAG: OpExtInst %[[V3_INT]] {{.*}} SMin
// EXTREMA-SPIRV-DAG: OpExtInst %[[V3_INT]] {{.*}} SMax
// EXTREMA-SPIRV-DAG: OpExtInst %[[V2_UINT]] {{.*}} UMin
// EXTREMA-SPIRV-DAG: OpExtInst %[[V2_UINT]] {{.*}} UMax

// EXTREMA_SPIRV_NO_ARTIFACT-NOT: INumericExtrema
// EXTREMA_SPIRV_NO_ARTIFACT-NOT: __numericsBuiltin
// SPIRV: OpDot
// SPIRV-NOT: __numericsBuiltinDot
// SPIRV-NOT: IDotProduct
// SPIRV: OpExtInst {{.*}} Sin
// SPIRV-NOT: __numericInterface
// SPIRV-NOT: IElementaryFunctions
// SPIRV-NOT: IReal
// SPIRV: OpExtInst {{.*}} Exp
// SPIRV-NOT: __numericInterface
// SPIRV-NOT: IElementaryFunctions
// SPIRV-NOT: IReal

import slang.numerics;

RWStructuredBuffer<float4> outputBuffer;
StructuredBuffer<float4> floatingInput;
StructuredBuffer<int4> integerInput;
StructuredBuffer<uint4> unsignedInput;

T floatingCurve<T : IFloatingPoint & IReal>(T x, T y)
{
    T elementary = sin(x) + exp(x) + sqrt(x) + powr(x, y);
    return min(elementary, max(divide(x, y), rcp(y)));
}

T integerCurve<T : IIntegerType>(T x, T y)
{
    return (x / y + x % y) ^ (x | y);
}

T.Mask orderedMask<T : IComponentwiseOrdered>(T x, T y)
{
    return x < y;
}

T reciprocal<T : IFractional>(T value)
{
    return rcp(value);
}

T.Scalar dotProduct<T : IDotProduct>(T left, T right)
{
    return dot(left, right);
}

[ForceInline]
T interfaceMinimum<T : INumericExtrema>(T left, T right)
{
    return min(left, right);
}

[ForceInline]
T interfaceMaximum<T : INumericExtrema>(T left, T right)
{
    return max(left, right);
}

typealias RowMajorUInt2x2 = matrix<uint, 2, 2, MatrixLayoutMode.RowMajor>;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float4 floatingResult = floatingCurve(floatingInput[0], floatingInput[1]);
    float2x2 matrixReciprocal = reciprocal(float2x2(2.0));
    int4 integerResult = integerCurve(integerInput[0], integerInput[1]);
    bool ordered = orderedMask(floatingInput[0], floatingInput[1]).all();
    float dotted = dotProduct(floatingInput[0], floatingInput[1]);
    outputBuffer[0] =
        floatingResult + float4(integerResult) + float(ordered) + matrixReciprocal[0][0] + dotted;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void extremaMain()
{
    float scalarMinimum = interfaceMinimum(floatingInput[0].x, floatingInput[1].x);
    float scalarMaximum = interfaceMaximum(floatingInput[0].y, floatingInput[1].y);

    int3 vectorMinimum = interfaceMinimum(integerInput[0].xyz, integerInput[1].xyz);
    int3 vectorMaximum = interfaceMaximum(integerInput[0].yzw, integerInput[1].yzw);

    RowMajorUInt2x2 matrixLeft = RowMajorUInt2x2(unsignedInput[0]);
    RowMajorUInt2x2 matrixRight = RowMajorUInt2x2(unsignedInput[1]);
    RowMajorUInt2x2 matrixMinimum = interfaceMinimum(matrixLeft, matrixRight);
    RowMajorUInt2x2 matrixMaximum = interfaceMaximum(matrixLeft, matrixRight);

    outputBuffer[0] = float4(scalarMinimum, scalarMaximum, vectorMinimum.xy);
    outputBuffer[1] = float4(vectorMinimum.z, vectorMaximum);
    outputBuffer[2] = float4(matrixMinimum[0], matrixMinimum[1]);
    outputBuffer[3] = float4(matrixMaximum[0], matrixMaximum[1]);
}
