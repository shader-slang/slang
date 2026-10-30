//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// Requirements an optional constraint inherits stay usable where the constraint is checked or
// where a required constraint also proves them (shader-slang/slang#13011, #13061).

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct NotArithmetic
{
    int x;
}

// `add` is an `IArithmetic` requirement, which `IFloat` inherits.
T addIfFloat<T>(T a, T b) where optional T : IFloat
{
    if (T is IFloat)
        return a.add(b);
    return a;
}

U twice<U : IArithmetic>(U x)
{
    return x.add(x);
}

// Only instantiated with `float`: with a type that is not `IFloat`, the guarded call still aborts
// in IR specialization (E99997), which shader-slang/slang#13011 tracks.
T twiceIfFloat<T>(T a) where optional T : IFloat
{
    if (T is IFloat)
        return twice(a);
    return a;
}

T modIfInteger<T>(T a, T b) where optional T : __BuiltinIntegerType
{
    if (T is __BuiltinIntegerType)
        return a % b;
    return a;
}

// Both constraints lead to `IArithmetic`; the required `IInteger` one proves it, whichever
// order the constraints are written in.
T addOptionalFirst<T>(T a, T b)
    where optional T : IFloat
    where T : IInteger
{
    return a + b;
}

T addRequiredFirst<T>(T a, T b)
    where T : IInteger
    where optional T : IFloat
{
    return a + b;
}

// The required `__BuiltinIntegerType` proof of `T : IArithmetic` is kept even though the path
// through it is less direct than the one through the optional `IFloat`.
T twiceRequiredInteger<T>(T a)
    where optional T : IFloat
    where T : __BuiltinIntegerType
{
    return twice(a);
}

interface IOne
{
    int one();
}

extension<U : IArithmetic> U : IOne
{
    int one() { return 1; }
}

// An extension on an interface that both constraints lead to applies in either order.
int oneOptionalFirst<T>(T t)
    where optional T : IFloat
    where T : IInteger
{
    return t.one();
}

int oneRequiredFirst<T>(T t)
    where T : IInteger
    where optional T : IFloat
{
    return t.one();
}

// The required proof of `IBase` comes from an extension, so it is found one round after the
// optional one, and extensions on `IBase` are considered again with it.
interface IBase
{
    int base();
}

interface IOptBase : IBase
{
}

interface IReq
{
}

interface IMid : IBase
{
}

extension<U : IReq> U : IMid
{
    int base() { return 2; }
}

interface ITwo
{
    int two();
}

extension<U : IBase> U : ITwo
{
    int two() { return 2; }
}

struct ReqImpl : IReq
{
}

int twoThroughExtension<T>(T t)
    where optional T : IOptBase
    where T : IReq
{
    return t.two();
}

T twiceRequiredIntegerFirst<T>(T a)
    where T : __BuiltinIntegerType
    where optional T : IFloat
{
    return twice(a);
}

// An initializer found through a checked optional constraint stays usable.
T makeIfArithmetic<T>(T a) where optional T : IArithmetic
{
    if (T is IArithmetic)
        return T(9);
    return a;
}

// The same through an inherited `IArithmetic`. Only instantiated with `float`, for the reason
// given for `twiceIfFloat`.
T makeIfFloat<T>(T a) where optional T : IFloat
{
    if (T is IFloat)
        return T(9);
    return a;
}

// A checked optional constraint also unlocks what is reached through an associated type, a
// user-defined base interface, and a default initializer.
interface IGet
{
    int get();
}

interface IMake
{
    associatedtype A : IGet;
    A make();
}

interface IMakeSub : IMake
{
}

struct GetImpl : IGet
{
    int get() { return 3; }
}

struct MakeImpl : IMakeSub
{
    typealias A = GetImpl;
    GetImpl make() { return GetImpl(); }
}

int makeIfSub<T>(T t) where optional T : IMakeSub
{
    if (T is IMakeSub)
        return t.make().get();
    return 0;
}

int getIfMake<T>(T.A a) where optional T : IMake
{
    if (T is IMake)
        return a.get();
    return 0;
}

interface IPlus
{
    This plus(This o);
}

interface IPlusSub : IPlus
{
}

struct PlusImpl : IPlusSub
{
    int v;
    This plus(This o) { return { v + o.v }; }
}

// Only instantiated with a conforming type, for the reason given for `twiceIfFloat`.
T plusIfSub<T>(T a, T b) where optional T : IPlusSub
{
    if (T is IPlusSub)
        return a.plus(b);
    return a;
}

interface IResettable
{
    __init();
    int get();
}

struct Resettable : IResettable
{
    int v;
    __init() { v = 42; }
    int get() { return v; }
}

int resetIfChecked<T>() where optional T : IResettable
{
    if (T is IResettable)
    {
        T x;
        return x.get();
    }
    return -1;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    NotArithmetic n = { 5 };

    // CHECK: 7
    outputBuffer[0] = int(addIfFloat<float>(3.0, 4.0));
    // CHECK-NEXT: 5
    outputBuffer[1] = addIfFloat<NotArithmetic>(n, n).x;
    // CHECK-NEXT: 2
    outputBuffer[2] = modIfInteger<int>(17, 5);
    // CHECK-NEXT: 5
    outputBuffer[3] = modIfInteger<NotArithmetic>(n, n).x;
    // CHECK-NEXT: 7
    outputBuffer[4] = addOptionalFirst<int>(3, 4);
    // CHECK-NEXT: 7
    outputBuffer[5] = addRequiredFirst<int>(3, 4);
    // CHECK-NEXT: 6
    outputBuffer[6] = int(twiceIfFloat<float>(3.0));
    // CHECK-NEXT: 8
    outputBuffer[7] = twiceRequiredInteger<int>(4);
    // CHECK-NEXT: 1
    outputBuffer[8] = oneOptionalFirst<int>(0);
    // CHECK-NEXT: 1
    outputBuffer[9] = oneRequiredFirst<int>(0);
    // CHECK-NEXT: 9
    outputBuffer[10] = makeIfArithmetic<int>(2);
    // CHECK-NEXT: 5
    outputBuffer[11] = makeIfArithmetic<NotArithmetic>(n).x;
    // CHECK-NEXT: 9
    outputBuffer[12] = int(makeIfFloat<float>(1.0));
    // CHECK-NEXT: 2
    outputBuffer[13] = twoThroughExtension<ReqImpl>(ReqImpl());
    // CHECK-NEXT: 8
    outputBuffer[14] = twiceRequiredIntegerFirst<int>(4);
    // CHECK-NEXT: 3
    outputBuffer[15] = makeIfSub<MakeImpl>(MakeImpl());
    // CHECK-NEXT: 3
    outputBuffer[16] = getIfMake<MakeImpl>(GetImpl());
    PlusImpl p2 = { 2 };
    PlusImpl p5 = { 5 };
    // CHECK-NEXT: 7
    outputBuffer[17] = plusIfSub<PlusImpl>(p2, p5).v;
    // CHECK-NEXT: 42
    outputBuffer[18] = resetIfChecked<Resettable>();
    // CHECK-NEXT: -1
    outputBuffer[19] = resetIfChecked<NotArithmetic>();
}
