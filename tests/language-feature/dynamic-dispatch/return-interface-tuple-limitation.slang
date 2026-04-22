// Returning a Tuple<IFoo, int> from a dynamically dispatched interface method.
// Previously crashed the compiler during tuple lowering (assert failure:
// loweredTupleInfo in slang-ir-lower-tuple-types.cpp). Fixed by improvements
// to dispatch wrapper code generation. (#10172)

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT: type_conformance FactoryA:IFactory = 0
//TEST_INPUT: type_conformance FactoryB:IFactory = 1
//TEST_INPUT: type_conformance FooA:IFoo = 0
//TEST_INPUT: type_conformance FooB:IFoo = 1

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

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // FactoryA.createWithMeta() -> (FooB(5.0), 42)
    // FooB(5.0).eval() = 5.0 * 2.0 = 10.0, plus 42 = 52.0
    IFactory factoryA = createDynamicObject<IFactory>(uint(0), 0.0);
    Tuple<IFoo, int> pairA = factoryA.createWithMeta();
    outputBuffer[0] = pairA._0.eval() + float(pairA._1);
    // CHECK: 52.0

    // FactoryB.createWithMeta() -> (FooA(6.0), 99)
    // FooA(6.0).eval() = 6.0, plus 99 = 105.0
    IFactory factoryB = createDynamicObject<IFactory>(uint(1), 0.0);
    Tuple<IFoo, int> pairB = factoryB.createWithMeta();
    outputBuffer[1] = pairB._0.eval() + float(pairB._1);
    // CHECK: 105.0

    // Test extracting just the interface element
    outputBuffer[2] = pairA._0.eval();
    // CHECK: 10.0

    // Test extracting just the int element
    outputBuffer[3] = float(pairB._1);
    // CHECK: 99.0
}
