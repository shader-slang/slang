// inner<true> gives Conditional<int,true> a literal flag, but the int->Conditional conversion is
// formed inside an enclosing generic (outer<T> / Grid<T>); lowering it must not depend on whether the
// pass visits the type before the makeConditionalValue that produces it. shader-slang/slang#11782.
//
// Each scenario reads the conditional value back with .get() and returns it, so the
// makeConditionalValue/getConditionalValue are live (not DCE'd before the pass) and the check
// distinguishes "conditional lowered correctly" from "conditional was deleted".

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

// SPIRV: OpEntryPoint

//TEST_INPUT:ubuffer(data=[10 20 30 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void inner<let flag : bool>(const Conditional<int, flag> c, out int r)
{
    if (let x = c.get())
        r = x;
    else
        r = -1;
}
void outer<typename T>(const int v, out int r)
{
    inner<true>(v, r);
}

struct Grid<typename T>
{
    void innerMethod<let flag : bool>(const Conditional<int, flag> c, out int r)
    {
        if (let x = c.get())
            r = x;
        else
            r = -1;
    }
    void outerMethod(const int v, out int r) { innerMethod<true>(v, r); }
};

// A nested Conditional<Conditional<int,true>,true> exercises the recursive value-type unwrap.
Conditional<Conditional<int, true>, true> nested(int v)
{
    Conditional<int, true> i = v;
    return i;
}

// CHECK: 10
// CHECK-NEXT: 20
// CHECK-NEXT: 30
[numthreads(1, 1, 1)]
void computeMain()
{
    int r0;
    outer<int>(outputBuffer[0], r0);
    outputBuffer[0] = r0;

    Grid<int> grid;
    int r1;
    grid.outerMethod(outputBuffer[1], r1);
    outputBuffer[1] = r1;

    int r2 = -1;
    if (let outerVal = nested(outputBuffer[2]).get())
    {
        if (let innerVal = outerVal.get())
            r2 = innerVal;
    }
    outputBuffer[2] = r2;
}
