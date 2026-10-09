
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -specialize F -validate-ir -O0

// This test checks lambda capture when the body contains a generic method call.
// Generic-call disambiguation may check the receiver expression before the capture context exists,
// so later capture analysis must still find both outer variables.

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
    uint capturedIndex = 0;
    let lambda = (uint x, float value) =>
    {
        // `pLocal.bias[capturedIndex].unpack<float>()` is parsed before lambda-capture analysis
        // runs. Capture analysis must still record both `pLocal` and `capturedIndex`, including the
        // variable used only as an index operand.
        pLocal.output[x] = pLocal.bias[capturedIndex].unpack<float>();
        return value;
    };
    invoke(lambda, 0, 1.0f);
}

[noinline]
void invoke(IFunc<float, uint, float> f, uint x, float value)
{
    f(x, value);
}

// The lambda struct captures both `pLocal` and the variable used as an index.
// CHECK: OpName %[[LAMBDA_STRUCT:[A-Za-z0-9_]+]] "computeMain._slang_Lambda_computeMain_1"
// CHECK: OpMemberName %[[LAMBDA_STRUCT]] 0 "pLocal"
// CHECK: OpMemberName %[[LAMBDA_STRUCT]] 1 "capturedIndex"

// Verify the lambda's operator() takes the lambda struct as its first parameter (`this`).
// CHECK: OpName %[[LAMBDA_FUNC:[A-Za-z0-9_]+]] "computeMain._slang_Lambda_computeMain_1.()"
// CHECK: %[[LAMBDA_FUNC]] = OpFunction

// Inside the lambda body, verify we access pLocal through the lambda struct (this),
// not directly from the parent function. The access chain should go through
// the lambda struct's `this` parameter to reach pLocal's fields.
// CHECK: OpAccessChain %{{.*}} %this_0 %int_0
