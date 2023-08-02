//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct B : IDifferentiable
{
    // CHECK:      tests/diagnostics/differential-type-constraint.slang([[#@LINE-2]]): error 30096: type 'B' is used as a `Differential` type, therefore it must serve as its own `Differential` type.
    // CHECK-NEXT: struct B : IDifferentiable
    // CHECK-NEXT:        ^
    float x;
};

struct A : IDifferentiable
{
    // CHECK:      tests/diagnostics/differential-type-constraint.slang([[#@LINE-2]]): note: see use of 'B' as Differential of 'IDifferentiable'.
    // CHECK-NEXT: struct A : IDifferentiable
    // CHECK-NEXT:            ^~~~~~~~~~~~~~~
    typedef B Differential;

    [DerivativeMember(B.x)]
    float x;
    float y;
};

typedef DifferentialPair<A> dpA;

[ForwardDifferentiable]
A f(A a)
{
    A aout;
    aout.y = detach(2 * a.x);
    aout.x = 5 * a.x;

    return aout;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        A a = {1.0, 2.0};
        B b = {0.2};

        dpA dpa = dpA(a, b);

        outputBuffer[0] = __fwd_diff(f)(dpa).d.x;
    }
}
