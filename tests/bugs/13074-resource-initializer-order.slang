// `cachedTexture` is initialized after `before` and before `after`. Moving its initializer into
// `main` would move its increment past `after`. We reject the resource initializer rather than
// change that observable order.
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
//CHECK:                 ^^^^^^^^^^^^^ initializer for a resource declared static at file or namespace scope cannot be moved safely
//CHECK:                 ^^^^^^^^^^^^^ Slang cannot prove that moving the initializer for resource 'cachedTexture', which is declared static at file or namespace scope, to each entry point preserves observable behavior. The initializer may have externally observable side effects or may read preexisting mutable storage, resource contents, or non-resource data from a source-declared parameter group; initialize the resource explicitly in an entry point instead

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
