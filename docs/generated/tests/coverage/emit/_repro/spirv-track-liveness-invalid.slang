// Repro for finding spirv-track-liveness-malformed-oplifetimestart.
// NOT a test (lives in _repro/, outside the bundle's *.slang glob).
// Compiling any shader with a local whose live range is tracked, using
// `-target spirv-asm -track-liveness`, emits an OpLifetimeStart of 5 words
// where the SPIR-V grammar allows 3, and spirv-opt rejects the module.
// The same file compiles cleanly without `-track-liveness`.
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float local = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
        float tmp = float(i) * 2.0f;
        local += tmp;
    }
    outputBuffer[tid.x] = local;
}
