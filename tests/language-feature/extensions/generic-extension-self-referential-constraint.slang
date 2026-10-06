//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// A blanket extension whose `where` clause names a generic interface applied to
// the extended type itself (`T : I<T>`). Computing the inheritance of a conforming
// type applies the extension to the type's interface bases too, and that
// application must terminate (gh-13461).

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

interface IComparable<T>
{
    bool lessThan(T other);
}

interface IEquatable<T>
{
    bool eq(T other);
}

extension<T> T : IEquatable<T>
    where T : IComparable<T>
{
    bool eq(T other) { return !this.lessThan(other) && !other.lessThan(this); }
}

struct Num : IComparable<Num>
{
    int v;
    bool lessThan(Num o) { return v < o.v; }
}

interface IRec<X>
{
}

interface IOther<X>
{
}

extension<T> T : IRec<T>
    where T : IOther<T>
{
}

interface IPlain
{
}

extension<T> T : IPlain
    where T : IOther<T>
{
}

struct Z : IOther<Z>
{
}

interface ISelf<X>
{
}

extension<T> T : ISelf<T>
    where T : ISelf<T>
{
}

struct W : ISelf<W>
{
}

int useRec<T>(T t)
    where T : IRec<T>
{
    return 1;
}

int usePlain<T : IPlain>(T t)
{
    return 2;
}

int useSelf<T>(T t)
    where T : ISelf<T>
{
    return 3;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Num a = { 1 };
    Num b = { 1 };
    Num c = { 2 };
    Z z;
    W w;

    // CHECK: 1
    outputBuffer[0] = a.eq(b) ? 1 : 0;
    // CHECK-NEXT: 0
    outputBuffer[1] = a.eq(c) ? 1 : 0;
    // CHECK-NEXT: 3
    outputBuffer[2] = useRec(z) + usePlain(z);
    // CHECK-NEXT: 3
    outputBuffer[3] = useSelf(w);
}
