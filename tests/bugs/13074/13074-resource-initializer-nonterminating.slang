// `cachedTexture` is initialized before `after`, whose initializer writes `output[0]`. If the first
// initializer cannot return, the original order prevents that write. Moving the resource
// initializer into `main` could instead make the write observable, so we reject the move unless the
// initializer and every function it can call return normally for all possible inputs.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -O0 -validate-ir

Texture2D<float4> inputTexture;
RWStructuredBuffer<uint> output;

[noinline]
Texture2D<float4> initializeTexture()
{
    for (;;)
    {
    }
}

static Texture2D<float4> cachedTexture = initializeTexture();
//CHECK:                 ^^^^^^^^^^^^^ cannot prove that a mutable resource initializer can be moved safely
//CHECK:                 ^^^^^^^^^^^^^ Slang cannot prove that moving the initializer for the mutable resource variable 'cachedTexture' to each entry point preserves observable behavior. The initializer may fail to return normally, may have externally observable side effects, or may read preexisting mutable storage, resource contents, or non-resource data from an explicit parameter group; initialize the resource explicitly in an entry point instead

[noinline]
uint initializeAfter()
{
    output[0] = 1;
    return 0;
}

static uint after = initializeAfter();

[numthreads(1, 1, 1)]
void main()
{
    output[1] = after + uint(cachedTexture.Load(int3(0, 0, 0)).x);
}
