//DIAGNOSTIC_TEST:SIMPLE(diag=diag,non-exhaustive):

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;

interface IFoo
{
    static float bar1(float x);

    [PrimalSubstitute(bar1)]
/*diag:
     ^^^^^^^^^^^^^^^^ interface requirement cannot have derivative
     ^^^^^^^^^^^^^^^^ cannot associate an interface requirement with a derivative.
*/
    static float bar(float x);

    static DifferentialPair<float> dd(DifferentialPair<float> x);
}

__generic<let N:int>
float f(float x)
{
    return N*x*x;
}

[ForwardDerivative(IFoo.dd)]
/*diag:
 ^^^^^^^^^^^^^^^^^ cannot resolve derivative function
 ^^^^^^^^^^^^^^^^^ cannot resolve the custom derivative function
                        ^^ cannot reference interface requirement 'dd' through the interface itself; use a conforming type or value
*/
float bbb(float x);

[ForwardDerivativeOf(IFoo.bar)]
/*diag:
 ^^^^^^^^^^^^^^^^^^^ cannot resolve original function for derivative
 ^^^^^^^^^^^^^^^^^^^ cannot resolve the original function for the the custom derivative.
                          ^^^ cannot reference interface requirement 'bar' through the interface itself; use a conforming type or value
*/
DifferentialPair<float> dd1(DifferentialPair<float> x)
{
    return x;
}

[BackwardDerivativeOf(f)]
/*diag:
 ^^^^^^^^^^^^^^^^^^^^ cannot deduce generic arguments for derivative
*/
DifferentialPair<float> df<let N:int>(inout DifferentialPair<float> x, float dOut)
{
    var primal = x.p * x.p;
    var diff = 2 * x.p * x.d * N;
    return DifferentialPair<float>(primal, diff);
}
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(3.0, 1.0);
        outputBuffer[1] = __fwd_diff(f<3>)(dpa).d; // Expect: 6.0
    }
}
