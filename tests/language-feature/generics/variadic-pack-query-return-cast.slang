//TEST:INTERPRET(filecheck=CHECK):
// The compute run disables warning 30816 (inheritance-unstable, emitted by the `struct Derived : Base`
// declarations below); slangi does not accept that flag and does not surface the warning on its
// checked stream, so INTERPRET intentionally omits it.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -cpu -shaderobj -output-using-type -xslang -warnings-disable -xslang 30816

// Returning the result of `__first`/`__last` on a value pack through a `where`-clause witness must
// lower correctly for every witness shape the cast can take:
//   - a type-equality witness (`where T == int`): the selected element already has the return type,
//     so the cast is a representation-only no-op;
//   - a concrete-base witness (`where T == Derived`, returned as `Base`): the cast is a real field
//     projection applied to the already-selected element;
//   - a multi-level concrete-base witness (`where T == Leaf`, returned as `Base`): the pattern
//     witness is a `TransitiveSubtypeWitness`, exercising the `First`/`Last`->`Transitive` recursion.
// The per-position values are distinct so first-vs-last selection is not confused with the
// projection, and each `Derived`/`Leaf` sets fields other than the read one so a wrong-field slice
// (e.g. projecting `.y` instead of `.x`) would surface as the wrong number rather than passing.

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

// Multi-level hierarchy: `where T == Leaf` returning `Base` makes the pattern witness a
// `TransitiveSubtypeWitness` (Leaf->Mid->Base), so the First/Last arm recurses into the transitive
// path rather than a single `DeclaredSubtypeWitness`.
struct Mid : Base
{
    int m;
}
struct Leaf : Mid
{
    int l;
}
Base firstAsBaseMulti<each T>(expand each T args) where nonempty(T) where T == Leaf
{
    return __first(args);
}
Base lastAsBaseMulti<each T>(expand each T args) where nonempty(T) where T == Leaf
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
int runFirstAsBaseMulti()
{
    Leaf a;
    a.x = 7;
    a.m = 70;
    a.l = 700;
    Leaf b;
    b.x = 8;
    b.m = 80;
    b.l = 800;
    return firstAsBaseMulti(a, b).x;
}
int runLastAsBaseMulti()
{
    Leaf a;
    a.x = 7;
    a.m = 70;
    a.l = 700;
    Leaf b;
    b.x = 8;
    b.m = 80;
    b.l = 800;
    return lastAsBaseMulti(a, b).x;
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
    printf("%d\n", runFirstAsBaseMulti());
    // CHECK: 7
    printf("%d\n", runLastAsBaseMulti());
    // CHECK: 8
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
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
    outputBuffer[4] = runFirstAsBaseMulti();
    // BUF-NEXT: 7
    outputBuffer[5] = runLastAsBaseMulti();
    // BUF-NEXT: 8
}
