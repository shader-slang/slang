//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -specialize F -validate-ir -O0

// The parser classifies the generic member-access base below with read-only lexical and member
// lookup. It must not check or rewrite the expression while deciding whether `<` starts generic
// arguments. The definitive semantic check creates exactly one capture field for `pLocal`.

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
        // Both member accesses refer to the same local outside the lambda. The parser's read-only
        // generic-application probe leaves capture attachment to this definitive semantic check.
        pLocal.output[x] = pLocal.bias[0].unpack<float>();
        return value;
    };
    invoke(lambda, 0, 1.0f);
}

[noinline]
void invoke(IFunc<float, uint, float> f, uint x, float value)
{
    f(x, value);
}

// Semantic checking adds exactly one `pLocal` capture field to the lambda struct.
// CHECK: OpName %[[LAMBDA_STRUCT:[A-Za-z0-9_]+]] "computeMain._slang_Lambda_computeMain_1"
// CHECK: OpMemberName %[[LAMBDA_STRUCT]] [[PLOCAL_MEMBER:[0-9]+]] "pLocal"
// CHECK-NOT: OpMemberName %[[LAMBDA_STRUCT]] {{[0-9]+}} "pLocal"

// The lambda's `operator()` takes the lambda struct as its first parameter (`this`).
// CHECK: OpName %[[LAMBDA_FUNC:[A-Za-z0-9_]+]] "computeMain._slang_Lambda_computeMain_1.()"
// CHECK: %[[PLOCAL_INDEX:[A-Za-z0-9_]+]] = OpConstant %int [[PLOCAL_MEMBER]]
// CHECK: %[[LAMBDA_FUNC]] = OpFunction

// The lambda body accesses the captured local through the lambda struct (`this`) rather than
// directly through its parent function.
// CHECK: OpAccessChain %{{.*}} %this_0 %[[PLOCAL_INDEX]]
