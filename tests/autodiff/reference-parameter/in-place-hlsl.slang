// A whole thread-group-shared variable passed to a `__ref_readonly` parameter of a differentiated
// function is accessed in place, in each mode, both where the function reads an element itself and
// where it forwards the reference on. The derivatives read the element they need and never read or
// write the variable as a whole, since reading the whole variable races with the other invocations'
// writes to their own elements, and writing it back overwrites them.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute -experimental-feature -line-directive-mode none

RWStructuredBuffer<float> outputBuffer;
groupshared float g[4];

float plainRead(__ref_readonly float a[4], uint i)
{
    return a[i];
}

[Differentiable]
float readElement(no_diff __ref_readonly float a[4], uint i, float x)
{
    return a[i] * x;
}

[Differentiable]
float forwardToDifferentiable(no_diff __ref_readonly float a[4], uint i, float x)
{
    return readElement(a, i, x);
}

[Differentiable]
float forwardToNonDifferentiable(no_diff __ref_readonly float a[4], uint i, float x)
{
    return no_diff(plainRead(a, i)) * x;
}

[Differentiable]
float top(uint i, float x)
{
    return readElement(g, i, x) + forwardToDifferentiable(g, i, x) +
           forwardToNonDifferentiable(g, i, x);
}

[numthreads(4, 1, 1)]
void computeMain(uint i: SV_GroupIndex)
{
    var x = diffPair(1.0);
    bwd_diff(top)(i, x, 1.0);
    outputBuffer[i] = x.d + fwd_diff(top)(i, diffPair(1.0, 1.0)).d;
}

// CHECK-NOT: = g_0;
// CHECK-NOT: g_0 =
// CHECK: void computeMain
// CHECK-NOT: = g_0;
// CHECK-NOT: g_0 =
