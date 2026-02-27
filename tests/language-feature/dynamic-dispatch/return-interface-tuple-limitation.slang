// Returning a Tuple<IFoo, int> from a dynamically dispatched interface method
// crashes the compiler during tuple lowering (assert failure: loweredTupleInfo
// in slang-ir-lower-tuple-types.cpp). The dispatch code is correctly generated
// but code generation fails because the tuple lowering pass does not handle
// tuple types containing existential elements when they flow through dispatch
// wrapper functions.

// Disabled: ICE — SLANG_ASSERT(loweredTupleInfo) only fires in Debug builds;
// in Release, the null dereference is UB and won't produce error 99999. (#10172)
//DISABLE_TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -conformance "FactoryA:IFactory=0" -conformance "FactoryB:IFactory=1" -conformance "FooA:IFoo=0" -conformance "FooB:IFoo=1"

RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float eval();
}

interface IFactory
{
    Tuple<IFoo, int> createWithMeta();
}

struct FooA : IFoo
{
    float val;
    float eval() { return val; }
}

struct FooB : IFoo
{
    float val;
    float eval() { return val * 2.0; }
}

struct FactoryA : IFactory
{
    float dummy;

    Tuple<IFoo, int> createWithMeta()
    {
        Tuple<IFoo, int> result = { FooB(5.0), 42 };
        return result;
    }
}

struct FactoryB : IFactory
{
    float dummy;

    Tuple<IFoo, int> createWithMeta()
    {
        Tuple<IFoo, int> result = { FooA(6.0), 99 };
        return result;
    }
}

IFactory createFactory(uint id)
{
    return createDynamicObject<IFactory>(id, 0.0);
}

// CHECK: error 99999
[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IFactory factory = createFactory(uint(id));
    Tuple<IFoo, int> pair = factory.createWithMeta();
    outputBuffer[0] = pair._0.eval() + float(pair._1);
}
