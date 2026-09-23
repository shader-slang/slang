// A generic body is nested under `IRGeneric` and may be instantiated after resource-global
// legalization. The pass must diagnose the use instead of changing a function signature whose
// eventual call sites do not exist yet.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -entry main -stage fragment

Texture2D<float4> inputTexture;
static Texture2D<float4> cachedTexture = inputTexture;

export float4 useCached<T>(T value)
//CHECK:      ^^^^^^^^^ file-scope static resource requires rewritable direct calls
//CHECK:      ^^^^^^^^^ function 'useCached' accesses a file-scope static resource but may be invoked without an in-module direct call that can receive the resource as an added argument; pass the resource explicitly instead
{
    return cachedTexture.Load(int3(0, 0, 0));
}

float4 readCached()
{
    return cachedTexture.Load(int3(0, 0, 0));
}

export float4 callHelper<T>(T value)
//CHECK:      ^^^^^^^^^^ file-scope static resource requires rewritable direct calls
//CHECK:      ^^^^^^^^^^ function 'callHelper' accesses a file-scope static resource but may be invoked without an in-module direct call that can receive the resource as an added argument; pass the resource explicitly instead
{
    return readCached();
}

[shader("fragment")]
float4 main() : SV_Target
{
    return inputTexture.Load(int3(0, 0, 0));
}
