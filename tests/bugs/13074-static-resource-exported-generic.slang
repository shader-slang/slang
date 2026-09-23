// A generic body is nested under `IRGeneric` and may be instantiated after resource-global
// legalization. The pass must diagnose the use instead of changing a function signature whose
// eventual call sites do not exist yet.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -entry main -stage fragment

Texture2D<float4> inputTexture;
static Texture2D<float4> cachedTexture = inputTexture;

export float4 useCached<T>(T value)
//CHECK:      ^^^^^^^^^ resource declared static at file or namespace scope is unavailable to a function whose invocations cannot all be rewritten
//CHECK:      ^^^^^^^^^ function 'useCached' may be invoked without a direct call that Slang can rewrite, so Slang cannot automatically pass the resource that it accesses; pass the resource explicitly instead
{
    return cachedTexture.Load(int3(0, 0, 0));
}

float4 readCached()
{
    return cachedTexture.Load(int3(0, 0, 0));
}

export float4 callHelper<T>(T value)
//CHECK:      ^^^^^^^^^^ resource declared static at file or namespace scope is unavailable to a function whose invocations cannot all be rewritten
//CHECK:      ^^^^^^^^^^ function 'callHelper' may be invoked without a direct call that Slang can rewrite, so Slang cannot automatically pass the resource that it accesses; pass the resource explicitly instead
{
    return readCached();
}

[shader("fragment")]
float4 main() : SV_Target
{
    return inputTexture.Load(int3(0, 0, 0));
}
