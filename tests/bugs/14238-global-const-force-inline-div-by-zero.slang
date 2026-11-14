// A `[ForceInline]` helper that divides by a literal zero in a module-scope `static const`
// initializer is evaluated in the function that uses the constant, where constant propagation
// reports the division by zero at the call.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute

[__readNone]
[ForceInline]
int idiv(int a, int b)
{
    return a / b;
}

[__readNone]
[ForceInline]
uint urem(uint a, uint b)
{
    return a % b;
}

static const int kDiv = idiv(10, 0);
//CHECK:                    ^ divide by zero
static const uint kRem = urem(10, 0);
//CHECK:                     ^ divide by zero

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = kDiv + int(kRem);
}
