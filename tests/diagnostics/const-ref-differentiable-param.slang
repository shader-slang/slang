//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):


[Differentiable]
float f(__constref float3 val)
//CHECK:^^^^^^^^^^ 'borrow in' on differentiable parameter
//CHECK:^^^^^^^^^^ cannot use 'borrow in' on a differentiable parameter.
{
    return val.x;
}

struct MyType : IDifferentiable
{
    // Error: cannot use constref on a differentiable member method of a differentiable type.
    [Differentiable]
    [constref] float compute(float x) { return 0; }
/*CHECK:
     ^^^^^^^^ '[constref]' on differentiable member method
     ^^^^^^^^ cannot use '[constref]' on a differentiable member method of a differentiable type.
*/

    // OK
    [Differentiable]
    float compute1(float x) { return 0; }

    // OK
    [constref] float compute2(float x) { return 0;}
}

struct MyType2
{
    // OK.
    [Differentiable]
    [constref] float compute(float x) { return 0; }

    // OK
    [constref]
    float compute1(float x) { return 0; }
}
