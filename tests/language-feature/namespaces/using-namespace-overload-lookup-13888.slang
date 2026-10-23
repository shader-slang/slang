// Regression test for https://github.com/shader-slang/slang/issues/13888.
//
// Name lookup continues to outer scopes when the first scope level with hits contains an
// overloadable declaration, so that a call can still resolve to a function declared farther out.
// When no declaration at that level is overloadable, the declarations there hide same-named
// declarations in outer scopes. In a type or extension scope, the level also holds the inherited
// members, interface requirements and extension members that member lookup finds there.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// `A::f` (a variable) and `B::f` (a function) are found at one level. The level contains a
// function, so lookup also finds the global `f(float)`.
namespace A
{
    static const int f = 1;
}

namespace B
{
    int f(int x)
    {
        return x + 10;
    }
}

int f(float x)
{
    return 20;
}

namespace Outer
{
    namespace Inner
    {
        using namespace A;
        using namespace B;

        int callInt()
        {
            return f(1);
        }

        int callFloat()
        {
            return f(1.5f);
        }
    }

    // `addSiblingScopeForContainerDecl` puts the most recent `using namespace` first among the
    // peers, so here the variable `A::f` is the first hit. Lookup still continues because the
    // function `B::f` is on the same level.
    namespace Reversed
    {
        using namespace B;
        using namespace A;

        int callFloat()
        {
            return f(1.5f);
        }
    }
}

// `N` holds two overloads of `g`, so lookup continues and also finds the global variable `g`.
// Overload resolution still picks `N::g(int)`, as it did on master.
static const int g = 5;

namespace N
{
    int g(int x)
    {
        return x + 30;
    }

    int g(float x)
    {
        return 40;
    }

    int use()
    {
        return g(3);
    }
}

// Inside `S`, lookup of `Count` finds the member `S::Count` and the requirement `IFoo::Count` at
// one level. They hide the global `Count`, and the concrete member is preferred over the
// requirement. On master lookup also collected the global `Count`, and the reference was
// ambiguous (E39999).
static const int Count = 100;

interface IFoo
{
    static const int Count;
}

struct S : IFoo
{
    static const int Count = 4;

    int get()
    {
        return Count;
    }
}

// Inside an extension of `E<int>`, lookup of `K` finds the `K` of both extensions at one level.
// They hide the global `K`, and the less generic extension is preferred. On master lookup also
// collected the global `K`, which won as a non-extension declaration, so `get()` returned 100.
static const int K = 100;

struct E<T>
{
    T a;
}

extension<T> E<T>
{
    static const int K = 1;
}

extension E<int>
{
    static const int K = 2;

    int get()
    {
        return K;
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 11
    outputBuffer[0] = Outer.Inner.callInt();
    // CHECK: 20
    outputBuffer[1] = Outer.Inner.callFloat();
    // CHECK: 33
    outputBuffer[2] = N.use();

    S s;
    // CHECK: 4
    outputBuffer[3] = s.get();

    E<int> e;
    e.a = 0;
    // CHECK: 2
    outputBuffer[4] = e.get();
    // CHECK: 20
    outputBuffer[5] = Outer.Reversed.callFloat();
}
