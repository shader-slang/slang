// Regression for interface-requirement default arguments resolved through a
// conformance witness (shader-slang/slang#12700, #12640).

//TEST:INTERPRET(filecheck=CHECK):

interface IFooLiteral
{
    void func(int a = 7);
}

struct FooLiteral : IFooLiteral
{
    void func(int a) { printf("literal=%d\n", a); }
}

interface IFooThisDep
{
    static const int kDefault;
    void func(int a = kDefault);
}

struct FooThisDep : IFooThisDep
{
    static const int kDefault = 2;
    void func(int a) { printf("thisdep=%d\n", a); }
}

interface IFooCompound
{
    static const int kDefault;
    void func(int a = kDefault + 1);
}

struct FooCompound : IFooCompound
{
    static const int kDefault = 2;
    void func(int a) { printf("compound=%d\n", a); }
}

// When the implementation redeclares its own default, a concrete-type call must
// use the implementation's default, not the requirement's.
interface IFooOwn
{
    void func(int a = 7);
}

struct FooOwn : IFooOwn
{
    void func(int a = 99) { printf("own=%d\n", a); }
}

// A leading supplied argument followed by a trailing defaulted one exercises the
// default fallback at a non-zero parameter index.
interface IFooMulti
{
    static const int kDefault;
    void func(int a, int b = kDefault);
}

struct FooMulti : IFooMulti
{
    static const int kDefault = 5;
    void func(int a, int b) { printf("multi=%d,%d\n", a, b); }
}

// A generic caller invokes the defaulted requirement through a type parameter's
// interface bound, so the default is materialized while the receiver is still a
// this-type substitution.
void callThroughGeneric<T : IFooThisDep>(T t)
{
    t.func();
}

// Interface-bound dispatch of a value whose implementation redeclares its own
// default: the requirement's default (7) wins, not the implementation's (99).
void callOwnThroughGeneric<T : IFooOwn>(T t)
{
    t.func();
}

// When the requirement defaults BOTH parameters but the implementation redeclares
// only the trailing default, a zero-argument call binds to the requirement (the
// implementation needs one argument) yet lowers through the implementation, so
// EVERY omitted argument must come from the requirement — both a=1 and b=2 — never
// a per-parameter mix that would pick up the implementation's b=20.
interface IFooMixed
{
    static const int kA;
    static const int kB;
    void func(int a = kA, int b = kB);
}

struct FooMixed : IFooMixed
{
    static const int kA = 1;
    static const int kB = 2;
    void func(int a, int b = 20) { printf("mixed=%d,%d\n", a, b); }
}

// The same fix must cover a `__subscript` requirement whose defaulted parameter
// the implementation omits (a parallel lowering path from method calls).
interface IArr
{
    static const int kDefault;
    __subscript(int i, int extra = kDefault)->int
    {
        get;
    }
}

struct Arr : IArr
{
    static const int kDefault = 4;
    int data[8];
    __subscript(int i, int extra)->int
    {
        get { return data[i] + extra; }
    }
}

void main()
{
    FooLiteral a;
    a.func();
    //CHECK: literal=7

    FooThisDep b;
    b.func();
    //CHECK: thisdep=2

    FooCompound c;
    c.func();
    //CHECK: compound=3

    FooOwn d;
    d.func();
    //CHECK: own=99

    callOwnThroughGeneric(d);
    //CHECK: own=7

    FooMulti e;
    e.func(3);
    //CHECK: multi=3,5

    FooThisDep g;
    callThroughGeneric(g);
    //CHECK: thisdep=2

    FooMixed h;
    h.func();
    //CHECK: mixed=1,2

    Arr arr;
    arr.data[2] = 10;
    printf("subscript=%d\n", arr[2]);
    //CHECK: subscript=14
}
