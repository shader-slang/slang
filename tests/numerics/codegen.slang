// Verify that numeric-interface witnesses specialize to concrete target types and operations
// without retaining interface or witness artifacts.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -experimental-feature
//TEST:SIMPLE: -target metal -entry computeMain -stage compute -o numeric-interfaces-codegen.metal -experimental-feature
//TEST:SIMPLE: -target cuda -entry computeMain -stage compute -o numeric-interfaces-codegen.cuda.cpp -experimental-feature
//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -o numeric-interfaces-codegen.cpp -experimental-feature
//TEST:SIMPLE: -target wgsl -entry computeMain -stage compute -o numeric-interfaces-codegen.wgsl -experimental-feature

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

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float4 floatingResult = floatingCurve(floatingInput[0], floatingInput[1]);
    float2x2 matrixReciprocal = reciprocal(float2x2(2.0));
    int4 integerResult = integerCurve(integerInput[0], integerInput[1]);
    bool ordered = orderedMask(floatingInput[0], floatingInput[1]).all();
    outputBuffer[0] =
        floatingResult + float4(integerResult) + float(ordered) + matrixReciprocal[0][0];
}
