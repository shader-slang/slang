//TEST:INTERPRET(filecheck=CHECK):
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu -shaderobj -output-using-type -xslang -warnings-disable -xslang 30816

// Returning the result of `__first`/`__last` on a value pack through a `where`-clause witness must
// lower correctly for both witness shapes the cast can take:
//   - a type-equality witness (`where T == int`): the selected element already has the return type,
//     so the cast is a representation-only no-op;
//   - a concrete-base witness (`where T == Derived`, returned as `Base`): the cast is a real field
//     projection applied to the already-selected element.
// The distinct per-position values prove first-vs-last selection is not confused with the projection.

int firstEq<each T>(expand each T args) where nonempty(T) where T == int
{
    return __first(args);
}
int lastEq<each T>(expand each T args) where nonempty(T) where T == int
{
    return __last(args);
}

struct Base
{
    int x;
}
struct Derived : Base
{
    int y;
}
Base firstAsBase<each T>(expand each T args) where nonempty(T) where T == Derived
{
    return __first(args);
}
Base lastAsBase<each T>(expand each T args) where nonempty(T) where T == Derived
{
    return __last(args);
}

int runFirstEq()
{
    return firstEq(42, 7);
}
int runLastEq()
{
    return lastEq(42, 7);
}
int runFirstAsBase()
{
    Derived a;
    a.x = 5;
    a.y = 100;
    Derived b;
    b.x = 6;
    b.y = 200;
    return firstAsBase(a, b).x;
}
int runLastAsBase()
{
    Derived a;
    a.x = 5;
    a.y = 100;
    Derived b;
    b.x = 6;
    b.y = 200;
    return lastAsBase(a, b).x;
}

void main()
{
    printf("%d\n", runFirstEq());
    // CHECK: 42
    printf("%d\n", runLastEq());
    // CHECK: 7
    printf("%d\n", runFirstAsBase());
    // CHECK: 5
    printf("%d\n", runLastAsBase());
    // CHECK: 6
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = runFirstEq();
    // BUF: 42
    outputBuffer[1] = runLastEq();
    // BUF-NEXT: 7
    outputBuffer[2] = runFirstAsBase();
    // BUF-NEXT: 5
    outputBuffer[3] = runLastAsBase();
    // BUF-NEXT: 6
}
