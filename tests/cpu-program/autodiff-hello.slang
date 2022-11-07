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
    return dstd.exp(x);
}

[ForwardDifferentiable]
float myAbs(float x)
{
    return dstd.abs(x);
}

[ForwardDifferentiable]
float myPow(float x, float y)
{
    return dstd.pow(dstd.max(0.f, x), y);
}

[ForwardDifferentiable]
float mySqrt(float x)
{
    return dstd.sqrt(dstd.max(0.f, x));
}

void test0()
{
    printf("===== Test 0 =====\n");

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

    dx = dpfloat(-3.f, 2.f);
    dpfloat abs_res = __fwd_diff(myAbs)(dx);
    printf("abs: val = %.3f, grad = %.3f\n", abs_res.p(), abs_res.d());

    //dpfloat3 dx3 = dpfloat3(float3(2.f, -3.f, -1.f), float3(-2.f, 3.f, -1.f));
    //dpfloat3 abs_res3 = __fwd_diff(myAbs3)(dx3);
    //printf("val = (%.3f, %.3f, %.3f), grad = (%.3f, %.3f, %.3f)\n", abs_res3.p().x, abs_res3.p().y, abs_res3.p().z, abs_res3.d().x, abs_res3.d().y, abs_res3.d().z);
}

[ForwardDifferentiable]
float3 F(float3 f0, float3 f90, float cosTheta)
{
    return f0 + (f90 - f0) * dstd.pow(dstd.max(1 - cosTheta, 1e-6), 5);
}

[ForwardDifferentiable]
float evalLambdaGGX(float alphaSqr, float cosTheta)
{
    if (cosTheta <= 0) return 0;
    float cosThetaSqr = cosTheta * cosTheta;
    float tanThetaSqr = dstd.max(1 - cosThetaSqr, 0) / cosThetaSqr;
    return 0.5 * (-1 + dstd.sqrt(1 + alphaSqr * tanThetaSqr));
}

[ForwardDifferentiable]
float G(float alpha, float cosThetaI, float cosThetaO)
{
    float alphaSqr = alpha * alpha;
    float lambdaI = evalLambdaGGX(alphaSqr, cosThetaI);
    float lambdaO = evalLambdaGGX(alphaSqr, cosThetaO);
    return 1.0 / (1.0 + lambdaI + lambdaO);
}

struct BSDFParameters : IDifferentiable
{
    float3 albedo;
    float roughness;
}

[ForwardDifferentiable]
float myDot(float3 x, float3 y)
{
    //return dstd.dot(x, y);
    float result = 0.f;
    result = x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
    /*
    for (int i = 0; i < 3; ++i)
        result = result + x[i] * y[i];
    */
    return result;
}

[ForwardDifferentiable]
float3 evalGGXDivByPDF(in float3 wi, in float3 wo, in float3 h, in BSDFParameters params)
{
    const float3 F0Color = params.albedo;
    let F90Color = float3(1.0, 1.0, 1.0);
    return F(F0Color, F90Color, dstd.dot(wi, h)) * G(params.roughness, wi.z, wo.z) * dstd.dot(wi, h) / (wi.z * h.z);
}

void test1()
{
    printf("===== Test 1 =====\n");
    {
        dpfloat3 diff_f0 = dpfloat3(float3(0.1f), float3(0.f));
        dpfloat3 diff_f90 = dpfloat3(float3(0.9f), float3(1.f));
        dpfloat diff_cosTheta = dpfloat(0.5f, 1.f);

        dpfloat3 res = __fwd_diff(F)(diff_f0, diff_f90, diff_cosTheta);
        printf("F val = (%.3f, %.3f, %.3f), grad = (%.3f, %.3f, %.3f)\n", res.p().x, res.p().y, res.p().z, res.d().x, res.d().y, res.d().z);
    }

    {
        dpfloat diff_alphaSqr = dpfloat(0.01f, 1.f);
        dpfloat diff_cosTheta = dpfloat(0.0f, 2.f);

        dpfloat res = __fwd_diff(evalLambdaGGX)(diff_alphaSqr, diff_cosTheta);
        printf("ggx val = %.3f, grad = %.3f\n", res.p(), res.d());
    }

    {
        dpfloat diff_alpha = dpfloat(0.1f, 1.f);
        dpfloat diff_cosThetaI = dpfloat(0.8f, 0.f);
        dpfloat diff_cosThetaO = dpfloat(0.7f, 0.f);

        dpfloat res = __fwd_diff(G)(diff_alpha, diff_cosThetaI, diff_cosThetaO);
        printf("G val = %.3f, grad = %.3f\n", res.p(), res.d());

        float G_res = G(diff_alpha.p(), diff_cosThetaI.p(), diff_cosThetaO.p());
        printf("G val = %.3f\n", G_res);
    }

    {
        dpfloat3 diff_wi = dpfloat3(float3(-0.5f, 0.7f, 0.8f), float3(0.f, 0.f, 1.f));
        dpfloat3 diff_h = dpfloat3(float3(0.f, 0.f, 1.f), float3(0.f, 0.f, 2.f));

        dpfloat res = __fwd_diff(myDot)(diff_wi, diff_h);
        printf("dot val = %.3f, grad = %.3f\n", res.p(), res.d());
    }

    {
        dpfloat3 diff_wi = dpfloat3(float3(-0.5f, 0.7f, 0.8f), float3(0.f, 0.f, 1.f));
        dpfloat3 diff_wo = dpfloat3(float3(0.5f, -0.6f, 0.9f), float3(0.f, 0.f, 1.f));
        dpfloat3 diff_h = dpfloat3(float3(0.f, 0.f, 1.f), float3(0.f, 0.f, 0.f));
        DifferentialPair<BSDFParameters> diff_params = DifferentialPair<BSDFParameters>(
            { float3(0.9f, 0.9f, 0.9f), 0.2f }, { float3(0.f, 0.f, 0.f), 0.f });

        dpfloat3 res = __fwd_diff(evalGGXDivByPDF)(diff_wi, diff_wo, diff_h, diff_params);
        printf("eval val = (%.3f, %.3f, %.3f), grad = (%.3f, %.3f, %.3f)\n", res.p().x, res.p().y, res.p().z, res.d().x, res.d().y, res.d().z);

        float3 eval_res = evalGGXDivByPDF(diff_wi.p(), diff_wo.p(), diff_h.p(), diff_params.p());
        printf("eval val = (%.3f, %.3f, %.3f)\n", res.p().x, res.p().y, res.p().z);
    }
}

struct NonDiff
{
    float a;
}

[ForwardDifferentiable]
bool myFunc(NonDiff fIn, inout float x)
{
    x = dstd.pow(x, fIn.a);
    return (x > 1.f);
}

void test2(float a)
{
    printf("===== Test 2 =====\n");
    {
        NonDiff fIn = { a };
        dpfloat dpx = dpfloat(4.f, 1.f);
        bool res = __fwd_diff(myFunc)(fIn, dpx);
        printf("res = %d, val = %.3f, grad = %.3f\n", 1, dpx.p(), dpx.d());
    }
}

public __extern_cpp int main()
{
    test0();
    test1();
    test2(3.f);
    return 0;
}
