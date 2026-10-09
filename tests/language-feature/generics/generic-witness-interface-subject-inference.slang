//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Witness-constraint inference around interface-typed subjects (gh-13461).
// `getFromFoo(a)` and `getFromBar(b)` open their existential arguments, so `U`
// comes from the opened type's facets. Array arguments are not opened, so `T`
// is the interface type itself: `useBar(arr)` and `optionalDer(arr)` take the
// interface-subject path of witness inference. `getDefaulted` and
// `sameAsDefault` are equalities that narrow an interface-typed default to a
// concrete type.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

interface IBar<X>
{
    X v();
}

interface IFoo<X> : IBar<X>
{
}

struct S : IFoo<int>
{
    int v() { return 7; }
}

struct S5 : IFoo<int>
{
    int v() { return 5; }
}

U getFromFoo<T, U>(T t)
    where T : IFoo<U>
{
    return t.v();
}

U getFromBar<T, U>(T t)
    where T : IBar<U>
{
    return t.v();
}

int useBar<T>(T[2] t)
    where T : IBar<int>
{
    return 1;
}

interface IDer : IBar<int>
{
}

struct D : IDer
{
    int v() { return 9; }
}

int optionalDer<T>(T[2] t)
    where optional T : IDer
{
    if (T is IDer)
        return 1;
    return 2;
}

interface IGet
{
    int get();
}

struct G : IGet
{
    int get() { return 3; }
}

int getDefaulted<T = IGet>(int x)
    where T == G
{
    return T().get() + x;
}

int sameAsDefault<T, U = IFoo<int>>(T t)
    where U == T
{
    return 1;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo<int> a = S();
    IFoo<int> b = S5();
    S s;
    IFoo<int> arr[2] = { S(), S5() };
    IBar<int> bars[2] = { D(), D() };
    D ders[2] = { D(), D() };

    // CHECK: 7
    outputBuffer[0] = getFromFoo(a);
    // CHECK-NEXT: 5
    outputBuffer[1] = getFromBar(b);
    // CHECK-NEXT: 4
    outputBuffer[2] = getDefaulted(1);
    // CHECK-NEXT: 1
    outputBuffer[3] = sameAsDefault(s);
    // CHECK-NEXT: 1
    outputBuffer[4] = useBar(arr);
    // CHECK-NEXT: 2
    outputBuffer[5] = optionalDer(bars);
    // CHECK-NEXT: 1
    outputBuffer[6] = optionalDer(ders);
}
