// interface-shader-param-in-struct.slang

// This test puts interface-type shader parameters
// inside of structure types to make sure that works

//DISABLED_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute

//DISABLED_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -profile sm_6_0 -use-dxil
//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute

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


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out
RWStructuredBuffer<int> gOutputBuffer;

// Note: even though `C` doesn't include any
// uniform/ordinary dat as declared, it gets a
// constant buffer allocated for it because
// there is no way to rule out the possibility
// that it *will* contain uniform/ordinary data
// after specialization.
//
//TEST_INPUT:cbuffer(data=[0 0 0 0 0 0 0 0]):
cbuffer C
{
   IRandomNumberGenerationStrategy gStrategy;
}

struct Stuff
{
    IModifier modifier;
    int extra;
}

// Note: the data for global-scope existential parameters
// is being introduced *before* the entry point declaration,
// because the default policy used by `slangc` (which the
// render test also uses) is to specialize the parameters at
// the global scope (producing a new layout) and then compose
// that specialized global scope with the entry point.
//
// (The net result is that data related to global-scope
// specialization always precedes the data for entry point
// parameters in these tests today)
//
//TEST_INPUT: globalExistentialType MyStrategy
//TEST_INPUT:ubuffer(data=[1 2 4 8], stride=4):

[numthreads(4, 1, 1)]
void computeMain(
//TEST_INPUT:root_constants(data=[0 0 0 0 0 0 0 0 256]):
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
//TEST_INPUT:ubuffer(data=[16 32 64 128], stride=4):
