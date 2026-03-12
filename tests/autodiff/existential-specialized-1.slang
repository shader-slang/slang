// Test that specializing a differentiable function taking an interface-typed
// parameter does not lose derivative information.
//
// When a differentiable function accepts an interface-typed parameter, and the
// caller passes a concrete type directly, the compiler can see the exact type
// and may want to specialize/devirtualize the call. However, doing so too early
// (before autodiff) may change the identity of the callee, causing the
// derivative function mapping to be lost.

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IInterface : IDifferentiable
{
    [Differentiable]
    float calc(float x);
}

struct A : IInterface
{
    int data1;

    __init(int data1) { this.data1 = data1; }

    [Differentiable]
    float calc(float x) { return x * x * x * data1; }
};

struct B : IInterface
{
    int data1;
    int data2;

    __init(int data1, int data2) { this.data1 = data1; this.data2 = data2; }

    [Differentiable]
    float calc(float x) { return x * x * data1 * data2; }
};

// A differentiable function that takes an interface-typed parameter.
// The compiler could specialize this when the concrete type is known at the call site,
// but must preserve differentiability when doing so.
[Differentiable]
float doThing(IInterface obj, float x)
{
    return obj.calc(x);
}

// Calls doThing with a concrete A instance.
// The type flowing into the interface parameter is statically known,
// making this a candidate for specialization/devirtualization.
[Differentiable]
float callWithA(float x)
{
    let obj = A(2);
    return doThing(obj, x);
}

// Calls doThing with a concrete B instance.
[Differentiable]
float callWithB(float x)
{
    let obj = B(2, 3);
    return doThing(obj, x);
}

//TEST_INPUT: type_conformance A:IInterface = 0
//TEST_INPUT: type_conformance B:IInterface = 1

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // A.calc(x) = 2*x^3, derivative = 6*x^2
    // fwd_diff at x=1.0, dx=2.0: 6*1*2 = 12
    outputBuffer[0] = fwd_diff(callWithA)(DifferentialPair<float>(1.0, 2.0)).d;

    // B.calc(x) = 6*x^2, derivative = 12*x
    // fwd_diff at x=1.5, dx=1.0: 12*1.5*1 = 18
    outputBuffer[1] = fwd_diff(callWithB)(DifferentialPair<float>(1.5, 1.0)).d;

    // bwd_diff of A.calc at x=1.0, dout=2.0: 6*1*2 = 12
    {
        var dpx = diffPair(1.0);
        bwd_diff(callWithA)(dpx, 2.0);
        outputBuffer[2] = dpx.d;
    }

    // bwd_diff of B.calc at x=1.5, dout=1.0: 12*1.5*1 = 18
    {
        var dpx = diffPair(1.5);
        bwd_diff(callWithB)(dpx, 1.0);
        outputBuffer[3] = dpx.d;
    }
}

/*
// No dynamic dispatch should be emitted.
// After auto-diff, both calls should get specialized.
//
// REPORT-NOT: {{.*}} dispatch code {{.*}}

// OpSpecializeExistentials(doThing, param1Info, param2Info, ...)
// OpSpecializeExistentials(doThing, void, TaggedUnionType({A, B}, {...witnesses..}))
float doThing(float, IInterface obj) { }

void main()
{
    fwd_diff(doThing)(DifferentialPair<Impl>(Impl(0, 0)));
    // OpCall(OpForwardDifferentiate(doThing), pairVal);
    // OpSpecializeExistentials(doThing_fwd, DifferentialPair<Impl>)
}

void main1()
{
    let ab = factoryAB();
    doThing(ab, 0.f); // OpSpecializeExistentials(doThing, void, TaggedUnionType({A, B}, {...witnesses..}))


    // ... 
    let bc = factoryBC();
    doThing(bc, 0.f); // OpSpecializeExistentials(doThing, void, TaggedUnionType({B, C}, {...witnesses..}))

    doThing_fwd // DifferentialPair<Impl> -> float


    doThing_apply(float, IInterface obj)
     -> s_bwdCallableCtx_doThing

    doThing_backProp(s_bwdCallableCtx_doThing, out float outGradient)
}

