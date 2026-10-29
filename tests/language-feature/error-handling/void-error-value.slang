//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST:SIMPLE(filecheck=SPIRV): -entry computeMain -stage compute -target spirv
//TEST:SIMPLE(filecheck=HLSL): -entry computeMain -stage compute -target hlsl

// Regression for #14066. Keep the throwing calls out of line so the test exercises extraction
// from the physical Result<void, E> representation, rather than folding the throw into its catch.
// CHECK: 11
// CHECK-NEXT: 17
// CHECK-NEXT: 165
// CHECK-NEXT: 7B

// SPIRV: OpCompositeExtract
// SPIRV: OpFunction {{.*}} DontInline
// HLSL: struct ResultType
// HLSL: unpackAnyValue

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
void fail(uint code) throws uint
{
    throw code;
}

[noinline]
void forward(uint code) throws uint
{
    try fail(code);
}

struct Error
{
    uint code;
    uint detail;
}

[noinline]
void maybeFail(bool shouldFail, uint code) throws Error
{
    if (shouldFail)
        throw Error(code, 47);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    do
    {
        try fail(tid.x + 17);
    }
    catch (error : uint)
    {
        outputBuffer[0] = error;
    }
    do
    {
        try forward(tid.x + 23);
    }
    catch (error : uint)
    {
        outputBuffer[1] = error;
    }
    do
    {
        try maybeFail(true, tid.x + 31);
    }
    catch (error : Error)
    {
        outputBuffer[2] = error.code * 10 + error.detail;
    }
    do
    {
        try maybeFail(false, tid.x);
        outputBuffer[3] = 123;
    }
    catch (error : Error)
    {
        outputBuffer[3] = error.code;
    }
}
