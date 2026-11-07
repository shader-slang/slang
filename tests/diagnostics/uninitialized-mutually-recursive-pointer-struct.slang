// The uninitialized-value check handles a local whose struct type is mutually recursive with
// another struct through pointers (shader-slang/slang#13404).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain -stage compute

struct A
{
    float x;
    B* b;
};

struct B
{
    float y;
    A* a;
};

RWStructuredBuffer<float> outputBuffer;

static A neverWritten;

float readUninitialized()
{
    B b;
    return b.y;
/*CHK:
    ^^^^^^ use of uninitialized variable
    ^^^^^^ use of uninitialized variable 'b'
*/
}

[numthreads(1, 1, 1)]
void computeMain()
{
    A a;
    a.x = 1.0;
    a.b = nullptr;
    outputBuffer[0] = a.x + readUninitialized();
    outputBuffer[1] = neverWritten.x;
/*CHK:
                    ^ use of uninitialized global variable
                    ^ use of uninitialized global variable 'neverWritten'
*/
}
