// TEST:EXECUTABLE:
typedef DifferentialPair<float> dpfloat;
typedef DifferentialPair<float3> dpfloat3;

[ForwardDifferentiable]
float mySqr(float x)
{
    return x * x;
}

[ForwardDifferentiable]
float myExp(float x)
{
    return exp(x);
}

[ForwardDifferentiable]
float myAbs(float x)
{
    return abs(x);
}

[ForwardDifferentiable]
float myPow(float x, float y)
{
    return pow(max(0.f, x), y);
}

[ForwardDifferentiable]
float mySqrt(float x)
{
    return sqrt(max(0.f, x));
}

[ForwardDifferentiable]
float myDetach(float x)
{
    return detach(x);
}

void test0()
{
    printf("===== Test 0 =====\n");
    /*
    dpfloat dx = dpfloat(-5.f, 1.f);
    dpfloat res = __fwd_diff(mySqr)(dx);
    printf("sqr: val = %.3f, grad = %.3f\n", res.p(), res.d());

    dpfloat sqrt_res = __fwd_diff(mySqrt)(dx);
    printf("sqrt: val = %.3f, grad = %3.f\n", sqrt_res.p(), sqrt_res.d());

    dpfloat exp_res = __fwd_diff(myExp)(dx);
    printf("exp: val = %.3f, grad = %.3f\n", exp_res.p(), exp_res.d());

    dpfloat dy = dpfloat(3.f, 0.f);
    dpfloat pow_res = __fwd_diff(myPow)(dx, dy);
    printf("pow: val = %.3f, grad = %.3f\n", pow_res.p(), pow_res.d());
    */

    {
        dpfloat dx = dpfloat(-3.f, 2.f);
        dpfloat abs_res = __fwd_diff(myAbs)(dx);
        printf("abs: val = %.3f, grad = %.3f\n", abs_res.p, abs_res.d);
    }

    {
        dpfloat dx = dpfloat(10.f, 5.f);
        dpfloat detach_res = __fwd_diff(myDetach)(dx);
        printf("detach: val = %.3f, grad = %.3f\n", detach_res.p, detach_res.d);
    }

    //dpfloat3 dx3 = dpfloat3(float3(2.f, -3.f, -1.f), float3(-2.f, 3.f, -1.f));
    //dpfloat3 abs_res3 = __fwd_diff(myAbs3)(dx3);
    //printf("val = (%.3f, %.3f, %.3f), grad = (%.3f, %.3f, %.3f)\n", abs_res3.p().x, abs_res3.p().y, abs_res3.p().z, abs_res3.d().x, abs_res3.d().y, abs_res3.d().z);
}

public __extern_cpp int main()
{
    test0();
    return 0;
}
