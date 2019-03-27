// interface-shader-param3.slang

// This test builds on `interface-shader-param3.slang` by putting
// resources into the concrete types that satisfy interface-type
// shader parameters.
//

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute

// A lot of the setup is the same as for `interface-shader-param`,
// so look there if you want the comments.

interface IRandomNumberGenerator
{
    [mutating]
    int randomInt();
}

interface IRandomNumberGenerationStrategy
{
    associatedtype Generator : IRandomNumberGenerator;
    Generator makeGenerator(int seed);
}

interface IModifier
{
    int modify(int val);
}

int test(
    int                             seed,
    IRandomNumberGenerationStrategy inStrategy,
    IModifier                       modifier)
{
    let strategy = inStrategy;
    var generator = strategy.makeGenerator(seed);
    let unused = generator.randomInt();
    let val = generator.randomInt();
    let modifiedVal = modifier.modify(val);
    return modifiedVal;
}


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
RWStructuredBuffer<int> gOutputBuffer;

ConstantBuffer<IRandomNumberGenerationStrategy> gStrategy;

[numthreads(4, 1, 1)]
void computeMain(

// Similarly to the previous test, we are declaring two `uniform`
// paameters on the entry point, where one will be plugged in
// with a concrete type, and thus get laid out second.
//
    uniform IModifier   modifier,
    uniform int         extra,
//
// The uniform/ordinary data for these two parameters will end
// up in the constant buffers, so let's declare that. Unlike
// the previous test, the concrete type plugged in for `modifier`
// has no uniform/ordinary data, so we don't need to fill it in.
//
//TEST_INPUT:cbuffer(data=[256]):dxbinding(0),glbinding(2)

            uint3       dispatchThreadID : SV_DispatchThreadID)
{
    let tid = dispatchThreadID.x;

    let inputVal : int = tid;
    let outputVal = test(inputVal, gStrategy, modifier)
    	+ extra*extra;

    gOutputBuffer[tid] = outputVal;
}

// Okay, now we get to the part that is unique starting
// in this test: we add data to the concrete types
// that we will use as parameters.

struct MyStrategy : IRandomNumberGenerationStrategy
{
	RWStructuredBuffer<int> globalSeeds;

    struct Generator : IRandomNumberGenerator
    {
        int state;

        [mutating]
        int randomInt()
        {
            return state++;
        }
    }

    Generator makeGenerator(int seed)
    {
        Generator generator = { globalSeeds[seed] };
        return generator;
    }
}

struct MyModifier : IModifier
{
	RWStructuredBuffer<int> localModifiers;

    int modify(int val)
    {
        return val ^ localModifiers[val & 3];
    }
}

//TEST_INPUT: globalExistentialType MyStrategy
//TEST_INPUT: entryPointExistentialType MyModifier

// The concrete types we plug in for `gStrategy` and `modifier`
// have buffer resources in them, so we need to assign them
// data. The registers/bindings for these parameters will
// always come after all other shader parameters, and their
// relative order will match the relative order of their
// declarations in the global order that Slang uses for
// assigning bindings (all globals before all entry point parameters).
//
// Here's the data for `gStrategy`:
//
//TEST_INPUT:ubuffer(data=[1 2 4 8], stride=4):dxbinding(1),glbinding(1)
//
// Here's the data for `modifier`:
//
//TEST_INPUT:ubuffer(data=[16 32 64 128], stride=4):dxbinding(2),glbinding(3)
