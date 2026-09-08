//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// A declaration whose name is an operator symbol can be referenced in
// expression position with `operator <op>` -- bare, after `.`, or after `::`.
// The escape produces the operator-symbol Name ("+", "()") that ordinary
// lookup uses, exactly as if the symbol had been written directly; it is not
// part of the name (the decl is named "+", never "operator+").
// See shader-slang/slang#12971.

struct Foo
{
    int v;
}

Foo operator+(Foo a, Foo b)
{
    Foo r = {a.v + b.v};
    return r;
}

struct Vec
{
    int x;

    Vec operator+(Vec b)
    {
        Vec r = {x + b.x};
        return r;
    }

    [mutating]
    int operator()(int p)
    {
        x += p;
        return x;
    }
}

namespace ns
{
    // Distinct result (a.v*100 + b.v) so a namespace-qualified reference is
    // provably resolved to this overload rather than the global operator+.
    Foo operator+(Foo a, Foo b)
    {
        Foo r = {a.v * 100 + b.v};
        return r;
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo a = {3};
    Foo b = {4};

    outputBuffer[0] = operator+(a, b).v;

    Vec p = {5};
    Vec q = {6};
    outputBuffer[1] = p.operator+(q).x;

    Vec counter = {0};
    outputBuffer[2] = counter.operator()(5);

    // A namespace qualifier accepts both `.` and `::`; each must select ns::operator+ (304),
    // not the global operator+.
    outputBuffer[3] = ns.operator+(a, b).v;
    outputBuffer[4] = ns::operator+(a, b).v;

    // Sanity: the unqualified/infix spelling binds the global operator+ (7),
    // not ns::operator+ (304).
    outputBuffer[5] = (a + b).v;

    // CHECK: 7
    // CHECK-NEXT: 11
    // CHECK-NEXT: 5
    // CHECK-NEXT: 304
    // CHECK-NEXT: 304
    // CHECK-NEXT: 7
}
