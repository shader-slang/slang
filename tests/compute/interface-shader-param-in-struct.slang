// interface-shader-param-in-struct.slang

// This test puts interface-type shader parameters
// inside of structure types to make sure that works

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

cbuffer C
{
   IRandomNumberGenerationStrategy gStrategy;
}

//TEST_INPUT: globalExistentialType MyStrategy
//TEST_INPUT:ubuffer(data=[1 2 4 8], stride=4):dxbinding(1),glbinding(1)

struct Stuff
{
    IModifier modifier;
    int extra;
}

[numthreads(4, 1, 1)]
void computeMain(
//TEST_INPUT:cbuffer(data=[256]):dxbinding(0),glbinding(2)
    uniform Stuff stuff,

    uint3       dispatchThreadID : SV_DispatchThreadID)
{
    let tid = dispatchThreadID.x;

    let inputVal : int = tid;
    let outputVal = test(inputVal, gStrategy, stuff.modifier)
        + stuff.extra*stuff.extra;

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

//TEST_INPUT: entryPointExistentialType MyModifier
//TEST_INPUT:ubuffer(data=[16 32 64 128], stride=4):dxbinding(2),glbinding(3)
