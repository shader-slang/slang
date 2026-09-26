// Codegen + execution test for shader-slang/slang#13220: VOID-returning interface dispatch, plus the
// emitter's broader unreachable rendering.
//
// createDispatchFunc has a distinct branch for a void-returning dispatched method (it terminates the
// default arm with a bare terminator, not a value return). This test covers that branch: on CUDA and
// C++ the void dispatcher's dead default arm must render SLANG_PRELUDE_UNREACHABLE() inside a `void`
// function; HLSL keeps a defined default. A COMPARE_COMPUTE -cpu run confirms the void dispatch still
// executes correctly. `loopMain` separately covers that the emitter renders the marker for *any*
// proven-dead block (here the point after a non-returning infinite loop), not only dispatch defaults.
// The dispatched type is read from a buffer so the dispatcher is not resolved away at compile time.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=LOOP): -target cpp -stage compute -entry loopMain -line-directive-mode none
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=EXEC): -shaderobj -output-using-type -cpu -entry computeMain

interface ISink
{
    void put(uint idx);
}

//TEST_INPUT: set types = ubuffer(data=[0 1 0 1], stride=4)
StructuredBuffer<uint> types;
//TEST_INPUT: set outputs = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputs;

struct SinkA : ISink
{
    void put(uint idx) { outputs[idx] = 42u; }
}

struct SinkB : ISink
{
    void put(uint idx) { outputs[idx] = 99u; }
}

// Runtime type keeps the void dispatcher from being resolved away.
ISink makeSink(uint type)
{
    if (type == 0u)
        return SinkA();
    return SinkB();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    for (uint i = 0u; i < 4u; i++)
        makeSink(types[i]).put(i);

    // types = [0, 1, 0, 1] -> SinkA, SinkB, SinkA, SinkB
    //EXEC: 42
    //EXEC: 99
    //EXEC: 42
    //EXEC: 99
}

// A non-returning entry: control cannot fall through the infinite loop, so the emitter renders the
// marker for the (dispatch-unrelated) unreachable block that follows it.
[shader("compute")]
[numthreads(1, 1, 1)]
void loopMain()
{
    for (;;)
    {
    }
}

// The void dispatcher's dead default arm renders the marker on CUDA/C++ (inside a `void` function).
//CUDA: default:
//CUDA: SLANG_PRELUDE_UNREACHABLE();
//CPP: default:
//CPP: SLANG_PRELUDE_UNREACHABLE();

// HLSL keeps its defined default and never emits the marker.
//HLSL: numthreads
//HLSL-NOT: SLANG_PRELUDE_UNREACHABLE

// The emitter renders the marker for any proven-dead block, not only dispatch defaults — here the
// point after loopMain's infinite loop.
//LOOP: SLANG_PRELUDE_UNREACHABLE
