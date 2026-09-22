//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

static_assert(sizeof(int) == 4, "int should be four bytes");
static_assert(sizeof(int) == 4);

// The short-circuiting `||` gives the condition its own control flow, which the synthesized carrier
// function hosts correctly (it would be malformed directly under the module inst).
static_assert(sizeof(int) == 4 || sizeof(double) == 8, "int or double size mismatch");

struct S
{
    int a;
    int b;

    static_assert(sizeof(int) == 4);
}

static_assert(sizeof(S) == 8, "S should be eight bytes");

namespace N
{
    static_assert(true);
}

// A `static_assert` in an interface body is a compile-time check, not a requirement, so `Impl` still
// conforms without providing a member for it.
interface IHasValue
{
    static_assert(sizeof(int) == 4, "int should be four bytes");
    int getValue();
}

struct Impl : IHasValue
{
    int getValue() { return 7; }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    static_assert(sizeof(S) == 8, "S should be eight bytes");
    static_assert(1 + 1 == 2);

    Impl impl;
    outputBuffer[0] = 42 - impl.getValue() + 7;
    // CHECK: 42
}
