
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -specialize F -validate-ir -O0

// Test: lambda capture of outer-scope variables when the lambda body contains
// a generic method call. The two-phase parsing's generic disambiguation
// (tryParseGenericApp) can pre-check expressions before the lambda capture
// context is established, causing outer-scope variable references to not be
// captured. This test ensures the capture is applied retroactively.

interface IPackedVal
{
    T unpack<T:__BuiltinArithmeticType>();
}

struct F:IPackedVal
{
    float val;
    T unpack<T:__BuiltinArithmeticType>()
    {
        return __arithmetic_cast<T>(val);
    }
}

struct Params<TBias : IPackedVal>
{
    TBias* bias;
    float* output;
}

[numthreads(1, 1, 1)]
void computeMain<TBias: IPackedVal>(
    ConstantBuffer<Params<TBias>> params)
{
    Params<TBias> pLocal = params;
    let lambda = (uint x, float value) =>
    {
        // pLocal.bias[0].unpack<float>() involves a generic method call that
        // triggers tryParseGenericApp during parsing. This pre-checks the
        // expression chain containing pLocal before the lambda capture context
        // exists. The LambdaCaptureVisitor must retroactively fix the capture.
        pLocal.output[x] = pLocal.bias[0].unpack<float>();
        return value;
    };
    invoke(lambda, 0, 1.0f);
}

void invoke(IFunc<float, uint, float> f, uint x, float value)
{
    f(x, value);
}

// Verify the lambda struct captures pLocal as a member.
// CHECK: OpName %[[LAMBDA_STRUCT:[A-Za-z0-9_]+]] "computeMain._slang_Lambda_computeMain_1"
// CHECK: OpMemberName %[[LAMBDA_STRUCT]] 0 "pLocal"

// Verify the lambda's operator() takes the lambda struct as its first parameter (`this`).
// CHECK: OpName %[[LAMBDA_FUNC:[A-Za-z0-9_]+]] "computeMain._slang_Lambda_computeMain_1.()"
// CHECK: %[[LAMBDA_FUNC]] = OpFunction

// Inside the lambda body, verify we access pLocal through the lambda struct (this),
// not directly from the parent function. The access chain should go through
// the lambda struct's `this` parameter to reach pLocal's fields.
// CHECK: OpAccessChain %{{.*}} %this_0 %int_0
