// Repro for finding wgsl-unsupported-int-image-format-falls-back-to-float.
// NOT a test (lives in _repro/, outside the bundle's *.slang glob).
// An integer storage image whose [format(...)] WGSL does not support falls back
// to the float format rgba32float, so the emitted textureLoad yields f32 while
// the variable it initializes is declared i32. slangc exits 0 with a warning.
RWStructuredBuffer<int> outputBuffer;

[format("r16i")]
RWTexture2D<int> tex;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = tex[tid.xy];
}
