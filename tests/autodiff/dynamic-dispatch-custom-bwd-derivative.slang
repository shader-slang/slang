// Test calling differentiable function through dynamic dispatch.

// For the new auto-diff system, we don't support dynamic dispatch via generics. Use existentials instead.

//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IInterface
{
    static float calc(float x);
}

struct A : IInterface
{
    static float calc(float x) { return 1.0; }
};

struct B : IInterface
{
    static float calc(float x) { return 2.0; }
};

void dsqr<T:IInterface>(T obj, inout DifferentialPair<float> x, float dOut)
{
    float diff = 2.0 * x.p * dOut;
    updateDiff(x, diff);
}

[BackwardDerivative(dsqr)]
float sqr<T:IInterface>(T obj, float x)
{
    return no_diff(obj.calc(x)) + x * x;
}

// Use automatically differentiated outer function to triger the primal/propagate func generation logic
// on a function that has user provided backward derivative.
[BackwardDifferentiable]
float sqr_outter<T:IInterface>(T obj, float x)
{
    return sqr(obj, x);
}

//TEST_INPUT: type_conformance A:IInterface = 0
//TEST_INPUT: type_conformance B:IInterface = 1


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var obj = createDynamicObject<IInterface>(dispatchThreadID.x, 0); // A
    var p = DifferentialPair<float>(2.0, 1.0);
    bwd_diff(sqr_outter)(obj, p, 1.0); // A.calc, expect 4

    // %1 = sqr_outter<obj.type, obj.table>;
    // %2 = (sqr_outter : IBackwardDifferentiable<%1>)<obj.type, obj.table>;
    // %3 = %2.apply_bwd
    // %4 = %3(...args...);
    outputBuffer[0] = p.d;

    obj = createDynamicObject<IInterface>(dispatchThreadID.x + 1, 0); // B
    p = DifferentialPair<float>(1.5, 1.0);
    bwd_diff(sqr)(obj, p, 1.0);
    outputBuffer[1] = p.d; // B.calc, expect 3
}

// Current thinking: let's just do it via key-compression & figure out how it works with trying to store 
// the lookup-witness's returned type.
// 

// Representing dynamic-dispatch stuff as Box-Each types.
//
// Expr : Typing Judgement
//
// ForEachTypeOf(%interface : InterfaceType) : TypeEnumeratorType<%interface>
// ForEachTableOf(%interface : InterfaceType) : TableEnumeratorType<%interface, WitnessTableType<%interface>>
//
// Lookup(%type_key, inst : TableEnumeratorType<%interface, WitnessTableType<%other_interface : InterfaceType>>) : 
//     TypeEnumeratorType<%interface>
// Lookup(%table_key, inst : TableEnumeratorType<%interface, WitnessTableType<%other_interface : InterfaceType>>) : 
//     TableEnumeratorType<%interface, %other_interface[table_key].requirement_type>>
// Lookup(%func_key, inst : TableEnumeratorType<%interface, WitnessTableType<%other_interface : InterfaceType>>) : 
//     FuncEnumeratorType<%interface, %other_interface[func_key].requirement_type>>
//
// Box(%interface : InterfaceType, inst : TypeEnumeratorType(%interface)) : 
//      TypeKind
// Box(%interface : InterfaceType, inst : TableEnumeratorType(%interface, WitnessTableType<%other_interface>)) : 
//      WitnessTableType<%other_interface> -> WitnessTableType()
// Box(%interface : InterfaceType, inst : FuncEnumeratorType(%interface, %func_type)) : %func_type
//
// Box(Specialize(%some_generic_type, args : List<EnumeratorTypeBase<%interface, _>>) -> 
//        Specialize(%some_generic_type, [Box(arg) for arg in args]))
//
// Box(LookupWitness(func_key, inst : TableEnumeratorType<%interface>) -> Can not be lowered in general.
// 
// // Special-case
// FwdDiffFuncType(FuncTypeOf(Box(Lookup(func_key, (inst : TableEnumeratorType<%interface>))))) ->
// => FwdDiffFuncType(%interface[func_key].type)
//
// 
// Main issue right now: 
// ACTUALLY NOT AN ISSUE... 
// - How do we lower the IRWitnessTableType<..> properly so that the inside is a box type?
// For lowering '%interface', we can ensure that all requirement types are under a Box(%interface, ...) type.
// But what about something looked up from a witness table that is itself looked up from %interface? 
// It'll use the non-boxed type, because we're not representing it properly in the front-end.
// 
// We just need a legalization pass to make sure that the looked-up witness-table-type is boxed properly,
// by comparing the looked-up table's type with the type in the interface requirement entry & simply
// copy it over if they don't match.
//
/*
{
    interface %IFoo
    {
        InterfaceRequirementEntry("calc", float -> float);
        InterfaceRequirementEntry("calc : IForwardDifferentiable<This.calc>", 
            WitnessTableType(Specialize(IForwardDifferentiable, Lookup(Box(%IFoo), "calc"))));
    }

    generic IForwardDifferentiable
    {
        %FType = Param;
        return interface
        {
            InterfaceRequirementEntry("fwd_diff", FwdDiffFuncType(FType));
        }
    }

    void main(IFoo foo)
    {
        %1 : %IFoo = foo;
        %2 : WitnessTableType(%IFoo) = ExtractExistentialWitnessTable(%1);
        %3 :  = LookupWitness(%2, "calc : IForwardDifferentiable<This.calc>");
        %4 : = LookupWitness(%2, "calc : IForwardDifferentiable<This.calc>");
        %4 : = Call(%3, ...);
    }
}
*/