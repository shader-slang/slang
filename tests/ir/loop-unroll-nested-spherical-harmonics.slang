//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// The shape that motivated shader-slang/slang#12473: spherical-harmonics code iterating `m`
// over the band `-l..l` for each band `l`. Both the lower and the upper bound of the inner
// loop depend on the enclosing loop's induction variable, and the trip count differs per
// outer iteration, so this exercises the deferral on a real-world pattern rather than the
// reduced `j < i` case.

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float sum = 0.0f;
    [ForceUnroll]
    for (int l = 0; l <= 3; ++l)
    {
        [ForceUnroll]
        for (int m = -l; m <= l; ++m)
        {
            sum += float(l * l + l + m);
        }
    }
    outputBuffer[0] = sum;
}

// CHECK: void computeMain
// CHECK-NOT: for(
// CHECK-NOT: for (
