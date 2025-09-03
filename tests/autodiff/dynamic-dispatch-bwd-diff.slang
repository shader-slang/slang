// Test calling backward differentiable function through dynamic dispatch.

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// TODO: STOPPED HERE: HIT A WALL
// 
// We can't lower an associated type for a specialized interface easily, so dynamic dispatch
// doesn't work at all, since the resulting associated type is 'empty' leading to a 
// 16-byte default size, which doesn't match with the call-site.
// 

[anyValueSize(16)]
interface IInterface
{
    [BackwardDifferentiable]
    float calc(float x);
    // IForwardDifferentiable<IInterface.calc>
}

struct A : IInterface
{
    float a;
    [BackwardDifferentiable]
    [NoDiffThis]
    float calc(float x) { return a*x*x; }
    // IForwardDifferentiable<A.calc> -> IForwardDifferentiable<IInterface.calc>
};

struct B : IInterface
{
    float a;
    [BackwardDifferentiable]
    [NoDiffThis]
    float calc(float x) { return a*x*x*x; }
    // IForwardDifferentiable<B.calc> -> IForwardDifferentiable<IInterface.calc>
};

[BackwardDifferentiable]
float run(IInterface obj, float x)
{
    //
    // %obj : IInterface
    // %table1 : WitnessTableType(IInterface) = ExtractWitnessTable(%obj)
    // %calc :  = LookupWitness("calc", %table1)
    // %table_type : TypeKind = Specialize(IForwardDifferentiable, %calc)
    // %calc_table : WitnessTableType(%table_type) = LookupWitness("calc : IFwdDiff", %table1)
    //

    // Post dynamic dispatch, we have the following:
    //
    // %obj : Tuple<>
    // %table1 : WitnessTableIDType = TupleGetElement(%obj)
    // %calc :  = LookupWitness("calc", %table1)
    // %table_type : TypeKind = Specialize(IForwardDifferentiable, %calc)
    // %calc_table : WitnessTableIDType(..) = LookupTableID("calc : IFwdDiff", %table1)
    //

    // obj.calc.IForwardDifferentiable<obj.calc>.fwd_diff(...) -->
    // obj.calc.IForwardDifferentiable<IInterface.calc>.fwd_diff(...)
    return obj.calc(x);
}

IInterface getDynObject(uint id, float a)
{
    if (id == 0)
    {
        A obj;
        obj.a = a;
        return obj;
    }
    else
    {
        B obj;
        obj.a = a;
        return obj;
    }
}

//TEST_INPUT: type_conformance A:IInterface = 0
//TEST_INPUT: type_conformance B:IInterface = 1

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    //var obj = createDynamicObject<IInterface>(dispatchThreadID.x, 0.5f); // A
    var obj = getDynObject(dispatchThreadID.x, 0.5f);
    var p = diffPair(3.0);

    bwd_diff(run)(obj, p, 1.0f);
    outputBuffer[0] = p.d; // A.calc, expect 3

    //obj = createDynamicObject<IInterface>(dispatchThreadID.x + 1, 1.5f); // B
    obj = getDynObject(dispatchThreadID.x + 1, 1.5f);
    p = diffPair(3.0);
    bwd_diff(run)(obj, p, 1.0f);
    outputBuffer[1] = p.d; // B.calc, expect 40.5
}
