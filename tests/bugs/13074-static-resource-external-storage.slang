//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

RWStructuredBuffer<float4> output;

__extern_cpp static Texture2D<float4> cachedTexture;
//CHECK:                              ^^^^^^^^^^^^^ resource declared static at file or namespace scope must remain in module-scope storage
//CHECK:                              ^^^^^^^^^^^^^ resource 'cachedTexture' is declared static at file or namespace scope and must remain in module-scope storage because it is externally accessible or explicitly retained; Slang cannot replace it with function-local storage

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
