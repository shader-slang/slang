// Empty (marker) interface with no methods. Verifies the existential
// layout machinery works even when there are no methods to dispatch.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

interface IMarker
{
}

struct A : IMarker
{
    int x;
}

struct B : IMarker
{
    float y;
}

void acceptMarker(IMarker m)
{
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IMarker m;
    m = A(1);
    acceptMarker(m);
    m = B(2.0);
    acceptMarker(m);
    outputBuffer[0] = 42; // CHECK: 42
}
