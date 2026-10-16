//TEST:SIMPLE(filecheck=CHECK_PTX): -target ptx -entry computeMain -stage compute

// Guards the automatic `-pch` code path in the NVRTC driver: on NVRTC 12.8+ the driver adds `-pch`
// when the source reaches NVRTC as a leading `#include` (the tool/test prelude form). This is a
// smoke test that the `-target ptx` path still emits `computeMain`; `-pch` activation is
// environment-dependent (requires NVRTC 12.8+) and is deliberately NOT asserted here — the
// behavioral create/reuse/rebuild checks live in tools/slang-unit-test/unit-test-nvrtc-pch*.cpp.

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = float(tid.x);
}

// CHECK_PTX: .visible .entry computeMain
