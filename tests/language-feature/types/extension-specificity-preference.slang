// extension-specificity-preference.slang

// Test that direct conformance is preferred over extension-provided conformance.
// When there are multiple ways for a type to conform to an interface:
// 1. Direct implementation on the type wins.
// 2. A concrete extension on the type is next.
// 3. A generic extension is the fallback.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

interface IGetVal
{
    int getVal();
}

interface ITransform
{
    int transform();
}

struct MyStruct : IGetVal
{
    int v;
    int getVal() { return v; }
}

// Direct extension on MyStruct providing ITransform (returns v * 10)
extension MyStruct : ITransform
{
    int transform() { return this.getVal() * 10; }
}

// Generic extension on all IGetVal types providing ITransform (returns v + 1)
extension<T : IGetVal> T : ITransform
{
    int transform() { return this.getVal() + 1; }
}

int callTransformDirect<T : ITransform>(T val)
{
    return val.transform();
}

int callTransformViaConstraint<T : IGetVal>(T val)
{
    return val.transform();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    MyStruct s = { 5 };

    // Direct call: uses the concrete extension (5 * 10 = 50)
    // CHECK: 50
    outputBuffer[0] = s.transform();

    // Through ITransform constraint: uses concrete extension (50)
    // CHECK: 50
    outputBuffer[1] = callTransformDirect(s);

    // Through IGetVal constraint: uses generic extension (5 + 1 = 6)
    // CHECK: 6
    outputBuffer[2] = callTransformViaConstraint(s);
}
