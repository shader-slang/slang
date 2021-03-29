// interface-shader-param.slang

// Test using interface tops as top-level shader parameters
// (whether global, or on an entry point).

//DISABLED_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute

//DISABLED_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -profile sm_6_0 -use-dxil
//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute

// First we will define some fake interfaces for testing.
// Let's pretend we are doing some kind of random number
// generation, so we need an interface for a generator.
//
interface IRandomNumberGenerator
{
    [mutating]
    int randomInt();
}

// We want each shader thread to have its own generator,
// so what we actually pass to the shader is a "strategy"
// for random number generation, which has the generator
// as its associated type.
//
interface IRandomNumberGenerationStrategy
{
    associatedtype Generator : IRandomNumberGenerator;

    Generator makeGenerator(int seed);
}

// Finally, just to give us another interface type to pass
// in, we'll define an interface to modify the generated
// random number (e.g., to make them fit an expected
// distribution).
//
interface IModifier
{
    int modify(int val);
}

// Let's define a subroutine that will use these interfaces
// to do something mildly interesting.
//
int test(
    int                             seed,
    IRandomNumberGenerationStrategy inStrategy,
    IModifier                       modifier)
{
    // HACK: The compiler currently has a problem with
    // looking up the conformance witness for `inStrategy`
    // to the `IRandomNumberGenreationStrategy` interface,
    // because it requires creating an `ExtractExistentialSubtypeWitness`
    // which refers to the value bound in a `LetExpr` created
    // by `maybeOpenExistential`, but we have no guarantee that
    // the code will actually emit the logic to initialize
    // that `LetExpr`...
    //
    let strategy = inStrategy;

    var generator = strategy.makeGenerator(seed);

    let unused = generator.randomInt();
    let val = generator.randomInt();

    let modifiedVal = modifier.modify(val);

    return modifiedVal;
}

// The global-scope parameters for this example include
// some `uniform` parameters, and that will trigger
// the allocation of a global-scope constant buffer to
// hold them. Slang's layout rules mean that the buffer
// will be allocated before any other global-scope parameters.
//
// In this example, the buffer will not be needed after specialization,
// but we need to declare/allocate it here so that the application
// creates a descriptor table/set that matches what the shader
// signature expects.
//
//TEST_INPUT:cbuffer(data=[0], stride=4):name=gStrategy



// Now we'll define a shader entry point that will use
// these interfaces to define its behavior.
//
// We'll start with the buffer for writing the test output.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<int> gOutputBuffer;

// Now we'll define a global shader parameter for the
// random number generation strategy.
//
//__disabled__TEST_INPUT:object(type=MyStrategy):name=gStrategy
uniform IRandomNumberGenerationStrategy gStrategy;

// The other parameter (for the modifier) will be attached
// the entry point instead, so that we are testing both
// cases.
//
//__disabled__TEST_INPUT:object(type=MyModifier):name=modifier
[numthreads(4, 1, 1)]
void computeMain(
//TEST_INPUT:root_constants(data=[0], stride=4):
    uniform IModifier   modifier,
            uint3       dispatchThreadID : SV_DispatchThreadID)
{
    let tid = dispatchThreadID.x;

    let inputVal : int = tid;
    let outputVal = test(inputVal, gStrategy, modifier);

    gOutputBuffer[tid] = outputVal;
}

// Now that we've define all the logic of the entry point,
// we will define some concrete types that we can plug
// in for the interface-type parameters.


struct MyStrategy : IRandomNumberGenerationStrategy
{
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
        Generator generator = { seed };
        return generator;
    }
}

struct MyModifier : IModifier
{
    int modify(int val)
    {
        return val * 16;
    }
}

//TEST_INPUT: globalExistentialType MyStrategy
//TEST_INPUT: entryPointExistentialType MyModifier
