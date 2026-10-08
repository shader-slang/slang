//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute

// This test checks initialization across loops. The C++ target can represent local resource
// storage, so the expected analysis diagnostics are not replaced by a target-legality error.

Texture2D<float4> textures[2];
RWStructuredBuffer<float4> output;
uniform uint loopCount;
uniform bool selectTexture;

static Texture2D<float4> textureFromRuntimeLoop;
static Texture2D<float4> textureFromConditionalLoop;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // A runtime loop may execute zero times, so its unconditional body assignment does not
    // initialize the value on every path to the later read.
    for (uint i = 0; i < loopCount; ++i)
        textureFromRuntimeLoop = textures[0];

    float4 result = textureFromRuntimeLoop.Load(int3(0, 0, 0));
//CHECK:                                       ^ possible use of uninitialized variable
//CHECK:                                       ^ variable 'textureFromRuntimeLoop' may be uninitialized on some paths; it is only conditionally assigned

    [ForceUnroll]
    for (uint i = 0; i < 1; ++i)
    {
        // Even when the loop executes, its conditional body can leave the value uninitialized.
        if (selectTexture)
            textureFromConditionalLoop = textures[1];
    }

    output[0] = result + textureFromConditionalLoop.Load(int3(0, 0, 0));
//CHECK:                                                ^ possible use of uninitialized variable
//CHECK:                                                ^ variable 'textureFromConditionalLoop' may be uninitialized on some paths; it is only conditionally assigned
}
