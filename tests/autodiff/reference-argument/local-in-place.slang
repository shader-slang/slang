// A local variable passed by reference from differentiated code, to a non-differentiable function
// called under `no_diff`, behaves as if the callee accessed it in place, even when the callee
// reaches it through more than one parameter.
//
// We pass an element and a whole local array. The callee adds 10 to element `i`, and the caller
// then reads that element again, so the derivative with respect to `x` shows both the callee's
// result and the updated element. We also pass the same element to two reference parameters,
// where a copy per argument would hide the callee's write through one parameter from its read
// through the other.
//
// Finally, we pass a whole scalar local to several calls, and to both reference parameters of one
// call, and then read it. The backward pass needs each local to be written by at most one call,
// so address elimination gives every call its own copy of the local, shared by all of that call's
// arguments and stored back after the call.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float bumpElement(__ref float a)
{
    a += 10.0;
    return a;
}

float bumpArray(__ref float a[4], uint i)
{
    a[i] += 10.0;
    return a[(i + 1) % 4];
}

float bumpFirstReadSecond(__ref float a, __ref float b)
{
    a += 10.0;
    return b;
}

[Differentiable]
float element(uint i, float x)
{
    float t[4] = { 1.0, 2.0, 3.0, 4.0 };
    float r = no_diff(bumpElement(t[i]));
    return (r + t[i]) * x;
}

[Differentiable]
float array(uint i, float x)
{
    float t[4] = { 1.0, 2.0, 3.0, 4.0 };
    float r = no_diff(bumpArray(t, i));
    return (r + t[i]) * x;
}

[Differentiable]
float aliased(uint i, float x)
{
    float t[4] = { 1.0, 2.0, 3.0, 4.0 };
    float r = no_diff(bumpFirstReadSecond(t[i], t[i]));
    return (r + t[i]) * x;
}

[Differentiable]
float wholeTwice(uint i, float x)
{
    float a = float(i + 1);
    return no_diff(bumpFirstReadSecond(a, a)) * x + no_diff(bumpFirstReadSecond(a, a)) * x;
}

[Differentiable]
float wholeTwoWrites(uint i, float x)
{
    float a = float(i + 1);
    no_diff(bumpElement(a));
    no_diff(bumpElement(a));
    return a * x;
}

[Differentiable]
float wholeAliased(uint i, float x)
{
    float a = float(i + 1);
    float r = no_diff(bumpFirstReadSecond(a, a));
    return (r + a) * x;
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    var x = diffPair(1.0);
    bwd_diff(element)(i, x, 1.0);
    outputBuffer[i] = x.d;
    outputBuffer[4 + i] = fwd_diff(element)(i, diffPair(1.0, 1.0)).d;

    x = diffPair(1.0);
    bwd_diff(array)(i, x, 1.0);
    outputBuffer[8 + i] = x.d;
    outputBuffer[12 + i] = fwd_diff(array)(i, diffPair(1.0, 1.0)).d;

    x = diffPair(1.0);
    bwd_diff(aliased)(i, x, 1.0);
    outputBuffer[16 + i] = x.d;
    outputBuffer[20 + i] = fwd_diff(aliased)(i, diffPair(1.0, 1.0)).d;

    x = diffPair(1.0);
    bwd_diff(wholeTwice)(i, x, 1.0);
    outputBuffer[24 + i] = x.d;

    x = diffPair(1.0);
    bwd_diff(wholeTwoWrites)(i, x, 1.0);
    outputBuffer[28 + i] = x.d;

    x = diffPair(1.0);
    bwd_diff(wholeAliased)(i, x, 1.0);
    outputBuffer[32 + i] = x.d;
}

// CHECK: 22.0
// CHECK-NEXT: 24.0
// CHECK-NEXT: 26.0
// CHECK-NEXT: 28.0
// CHECK-NEXT: 22.0
// CHECK-NEXT: 24.0
// CHECK-NEXT: 26.0
// CHECK-NEXT: 28.0
// CHECK-NEXT: 13.0
// CHECK-NEXT: 15.0
// CHECK-NEXT: 17.0
// CHECK-NEXT: 15.0
// CHECK-NEXT: 13.0
// CHECK-NEXT: 15.0
// CHECK-NEXT: 17.0
// CHECK-NEXT: 15.0
// CHECK-NEXT: 22.0
// CHECK-NEXT: 24.0
// CHECK-NEXT: 26.0
// CHECK-NEXT: 28.0
// CHECK-NEXT: 22.0
// CHECK-NEXT: 24.0
// CHECK-NEXT: 26.0
// CHECK-NEXT: 28.0
// CHECK-NEXT: 32.0
// CHECK-NEXT: 34.0
// CHECK-NEXT: 36.0
// CHECK-NEXT: 38.0
// CHECK-NEXT: 21.0
// CHECK-NEXT: 22.0
// CHECK-NEXT: 23.0
// CHECK-NEXT: 24.0
// CHECK-NEXT: 22.0
// CHECK-NEXT: 24.0
// CHECK-NEXT: 26.0
// CHECK-NEXT: 28.0
