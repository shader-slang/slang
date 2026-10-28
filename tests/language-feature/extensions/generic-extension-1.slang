//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
interface IFoo
{
    int getVal();
}

interface IBar
{
    int getValPlusOne();
}

interface IBaz
{
    int getValPlusTwo();
}

struct MyInt { int v; }

extension MyInt : IFoo
{
    int getVal() { return v; }
}

// Checking this target establishes MyInt : IFoo before the following blanket
// extension registers on IFoo. That registration must invalidate the cached
// inheritance of MyInt through its IFoo dependency, so the later IBar and IBaz
// methods are visible when the entry point is checked.
struct RequireFoo<T : IFoo> {}
extension RequireFoo<MyInt> {}

// Since MyInt:IFoo, the following extension will make MyInt:IBar.
extension<T: IFoo> T : IBar
{
    int getValPlusOne() { return this.getVal() + 1; }
}

// Since MyInt:IBar, the following extension will make MyInt:IBaz.
extension<T: IBar> T : IBaz
{
    int getValPlusTwo() { return this.getValPlusOne() + 1; }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    MyInt v = {1};

    // Check that the extensions applied to MyInt correctly, i.e.
    // MyInt.getValPlusTwo() eixsts.

    // CHECK: 3
    outputBuffer[0] = v.getValPlusTwo();
}