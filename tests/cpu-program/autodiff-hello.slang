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
float myMax(float x, float y)
{
    return max(x, y);
}

[ForwardDifferentiable]
float myClampToZero(float x)
{
    return max(x, 0.f);
}

[ForwardDifferentiable]
float mySqrt(float x)
{
    return sqrt(max(x, 0.f));
}

[ForwardDifferentiable]
float myDetach(float x)
{
    return detach(x);
}

[ForwardDifferentiable]
float myDot(float3 x, float3 y)
{
    return dot(x, y);
    // float result = 0.f;
    // result = x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
    /*
    for (int i = 0; i < 3; ++i)
        result = result + x[i] * y[i];
    */
    // return result;
}

void test0()
{
    printf("===== Test 0 =====\n");

    {
        dpfloat dx = dpfloat(-5.f, 1.f);
        dpfloat dy = dpfloat(-3.f, -1.f);
        dpfloat max_res = __fwd_diff(myMax)(dx, dy);
        printf("max: val = %.3f, grad = %.3f\n", max_res.p, max_res.d);
    }

    {
        dpfloat dx = dpfloat(-5.f, 1.f);
        dpfloat clamp_to_zero_res = __fwd_diff(myClampToZero)(dx);
        printf("clamp to 0: val = %.3f, grad = %.3f\n", clamp_to_zero_res.p, clamp_to_zero_res.d);
    }

    {
        dpfloat dx = dpfloat(-5.f, 1.f);
        dpfloat res = __fwd_diff(mySqr)(dx);
        printf("sqr: val = %.3f, grad = %.3f\n", res.p, res.d);
    }

    {
        dpfloat dx = dpfloat(-9.f, 1.f);
        dpfloat sqrt_res = __fwd_diff(mySqrt)(dx);
        printf("sqrt: val = %.3f, grad = %.3f\n", sqrt_res.p, sqrt_res.d);
    }

    {
        dpfloat dx = dpfloat(4.f, 1.f);
        dpfloat exp_res = __fwd_diff(myExp)(dx);
        printf("exp: val = %.3f, grad = %.3f\n", exp_res.p, exp_res.d);
    }

    {
        dpfloat dx = dpfloat(5.f, 1.f);
        dpfloat dy = dpfloat(3.f, 0.f);
        dpfloat pow_res = __fwd_diff(myPow)(dx, dy);
        printf("pow: val = %.3f, grad = %.3f\n", pow_res.p, pow_res.d);
    }

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

    {
        dpfloat3 diff_wi = dpfloat3(float3(-0.5f, 0.7f, 0.8f), float3(0.f, 0.f, 1.f));
        dpfloat3 diff_h = dpfloat3(float3(0.f, 0.f, 1.f), float3(0.f, 0.f, 2.f));

        dpfloat res = __fwd_diff(myDot)(diff_wi, diff_h);
        printf("dot val = %.3f, grad = %.3f\n", res.p, res.d);
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
