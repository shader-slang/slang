//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// Declaring a blanket extension with a `T : I<T>` where-clause next to a type that
// satisfies it must not overflow the stack while computing that type's
// inheritance, even when nothing uses the extension (gh-13461).

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

struct Z : IOther<Z>
{
}

RWStructuredBuffer<int> outputBuffer;

// CHECK: computeMain
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
