// Regression test for shader-slang/slang#14722: an unscoped HLSL enum member
// used as the shift count of `<<`, in an enum initializer (the reported shape)
// and in a function body. The shift-count operand coerces to the enum's tag
// type through the concrete shift overloads, the same argument-coercion path
// that already lets `+` take an enum operand. Before the fix this failed with
// `no overload for '<<' applicable to arguments of type (int, Options)`.

// Positive (compile, GPU target): the shifts compile on the Vulkan path.
// Value verification lives in the .slang CPU leg and the HLSL leg below.
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -unscoped-enum

// Positive (emitted value): the shifts must const-fold to the correct results,
// not merely compile. Compiling to HLSL emits the folded stores, pinning the
// values (a wrong tag or a missed fold would change them).
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -unscoped-enum

enum Options { Shift = 6, Mask = 1 << Shift };

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = Mask;
    outputBuffer[1] = 1024 >> Shift;
    //HLSL: outputBuffer{{.*}} = 64L;
    //HLSL: outputBuffer{{.*}} = 16L;
}

// CHECK: OpEntryPoint
