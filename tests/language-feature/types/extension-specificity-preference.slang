// extension-specificity-preference.slang

// Test which ITransform conformance is selected for MyStruct when both a concrete
// extension (`extension MyStruct : ITransform`) and a blanket generic extension
// (`extension<T : IGetVal> T : ITransform`) supply one.
//
// The selection is by *witness visibility in the calling context*, not a global
// "concrete beats generic" ranking: whichever ITransform witness is statically in
// scope where `transform()` is resolved is the one used.
//   - When the concrete type MyStruct is known (a direct `s.transform()` call, or a
//     `<T : ITransform>` generic that carries MyStruct's own ITransform witness), the
//     concrete extension is visible -> `v * 10`.
//   - Inside a `<T : IGetVal>` generic, only the blanket `extension<T : IGetVal> T`
//     witness is in scope (the concrete MyStruct extension is not reachable for an
//     arbitrary IGetVal), so the generic extension is used -> `v + 1`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

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

int callTransformViaITransform<T : ITransform>(T val)
{
    return val.transform();
}

int callTransformViaIGetVal<T : IGetVal>(T val)
{
    return val.transform();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    MyStruct s = { 5 };

    // Concrete type known: concrete extension's witness is visible (5 * 10 = 50).
    // CHECK: 50
    outputBuffer[0] = s.transform();

    // Through a <T : ITransform> constraint, T carries MyStruct's own (concrete)
    // ITransform witness (50).
    // CHECK: 50
    outputBuffer[1] = callTransformViaITransform(s);

    // Through a <T : IGetVal> constraint, only the blanket generic extension's
    // witness is in scope (5 + 1 = 6).
    // CHECK: 6
    outputBuffer[2] = callTransformViaIGetVal(s);
}
