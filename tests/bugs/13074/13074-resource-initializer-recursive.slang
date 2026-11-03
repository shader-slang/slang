// `initializeTexture` has an acyclic CFG, but its self-call can prevent it from returning. The
// initializer for `after` writes `output[0]` and follows `cachedTexture` in source order. Moving the
// resource initializer to an entry point could make that write observable, so we reject the move
// when the call-graph analysis finds the recursive cycle.
// We disable non-essential validation so that the ordinary recursion diagnostic does not stop
// compilation before the initializer-safety analysis runs.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -O0 -validate-ir -disable-non-essential-validations

Texture2D<float4> inputTexture;
RWStructuredBuffer<float4> output;

[noinline]
Texture2D<float4> initializeTexture()
{
    return initializeTexture();
}

static Texture2D<float4> cachedTexture = initializeTexture();
//CHECK:                 ^^^^^^^^^^^^^ cannot prove that a mutable resource initializer can be moved safely
//CHECK:                 ^^^^^^^^^^^^^ Slang cannot prove that moving the initializer for the mutable resource variable 'cachedTexture' to each entry point preserves observable behavior. The initializer may fail to return normally, may have externally observable side effects, or may read preexisting mutable storage, resource contents, or non-resource data from an explicit parameter group; initialize the resource explicitly in an entry point instead

[noinline]
uint initializeAfter()
{
    output[0] = float4(1.0);
    return 0;
}

static uint after = initializeAfter();

[numthreads(1, 1, 1)]
void main()
{
    output[1] = cachedTexture.Load(int3(0, 0, 0)) + float(after);
}
