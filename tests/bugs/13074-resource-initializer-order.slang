// Moving `cachedTexture`'s initializer into `main` would move its increment past the initializers for
// `before` and `after`. We reject the resource initializer rather than change that observable order.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -O0 -validate-ir

static uint sequence = 0;

[noinline]
uint initializeBefore()
{
    return ++sequence;
}

static uint before = initializeBefore();

Texture2D<float4> inputTexture;

[noinline]
Texture2D<float4> initializeTexture()
{
    ++sequence;
    return inputTexture;
}

static Texture2D<float4> cachedTexture = initializeTexture();
//CHECK:                 ^^^^^^^^^^^^^ unsupported file-scope static resource initializer
//CHECK:                 ^^^^^^^^^^^^^ initializer for file-scope static resource 'cachedTexture' has a side effect or reads mutable state; initialize the resource explicitly in an entry point instead

[noinline]
uint initializeAfter()
{
    return ++sequence;
}

static uint after = initializeAfter();

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = before;
    output[1] = after;
    output[2] = sequence;
    output[3] = uint(cachedTexture.Load(int3(0, 0, 0)).x);
}
