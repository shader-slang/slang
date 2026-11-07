// An interface-typed field of a struct whose layout is fixed by a shader
// parameter keeps the existential's buffer layout, while a callee's
// `inout` / `out` parameter is specialized to a compact tagged union.
// Passing the field to such a parameter must convert between the two
// layouts, both into the callee and back out of it (#14152).
//
// The parameters live in constant buffers. Each test copies them into a
// local and passes the local's field by reference.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

[anyValueSize(8)]
interface IFoo
{
    int getValue();
    [mutating] void bump(int d);
}

export struct ImplA : IFoo
{
    int v;
    int getValue() { return v; }
    [mutating] void bump(int d) { v += d; }
}

export struct ImplB : IFoo
{
    int v;
    int getValue() { return v * 2; }
    [mutating] void bump(int d) { v += 10 * d; }
}

struct Params
{
    IFoo foo;
    int scale;
}

//TEST_INPUT:set gParamsA = new{ foo: new ImplA{ v: 5 }, scale: 3 }
ConstantBuffer<Params> gParamsA;

//TEST_INPUT:set gParamsB = new{ foo: new ImplB{ v: 4 }, scale: 2 }
ConstantBuffer<Params> gParamsB;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: globalExistentialType __Dynamic
//TEST_INPUT: globalExistentialType __Dynamic

// Reads through an `inout` parameter.
int readRef(inout IFoo f)
{
    return f.getValue();
}

// Mutates through an `inout` parameter: the change must reach the field.
void bumpRef(inout IFoo f, int d)
{
    f.bump(d);
}

// Replaces the value through an `out` parameter.
void copyOut(out IFoo dst, IFoo src)
{
    dst = src;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Params a = gParamsA;
    Params b = gParamsB;

    // CHECK: 5
    outputBuffer[0] = readRef(a.foo);
    // CHECK-NEXT: 8
    outputBuffer[1] = readRef(b.foo);

    // ImplA: 5 + 1 = 6, read back by value: 6 * 3 = 18
    bumpRef(a.foo, 1);
    // CHECK-NEXT: 18
    outputBuffer[2] = a.foo.getValue() * a.scale;

    // ImplB: 4 + 10 = 14, getValue = 28
    bumpRef(b.foo, 1);
    // CHECK-NEXT: 28
    outputBuffer[3] = b.foo.getValue();
    // The read through `inout` sees the written-back value too.
    // CHECK-NEXT: 28
    outputBuffer[4] = readRef(b.foo);

    // `a.foo` becomes the bumped ImplB (v = 14), getValue = 28.
    copyOut(a.foo, b.foo);
    // CHECK-NEXT: 28
    outputBuffer[5] = a.foo.getValue();
    // CHECK-NEXT: 3
    outputBuffer[6] = a.scale;

    // The constant buffers are unchanged.
    // CHECK-NEXT: 5
    outputBuffer[7] = gParamsA.foo.getValue();
}
