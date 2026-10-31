// A constructor of a struct that is mutually recursive with another struct through pointers is
// checked for uninitialized fields (shader-slang/slang#13404).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain -stage compute

struct C
{
    float x;
    D* d;

    __init() { x = 1.0; }
/*CHK:
    ^^^^^^ exiting constructor without initializing field
    ^^^^^^ exiting constructor without initializing field 'd'
*/
};

struct D
{
    float y;
    C* c;
};

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    C c = C();
    outputBuffer[0] = c.x;
}
