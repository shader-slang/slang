//DIAGNOSTIC_TEST:SIMPLE(filecheck=DEPRECATED): -std 2025 -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DEPRECATED): -std 2025 -target cpp -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DEPRECATED): -std 2025 -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DEPRECATED): -std 2025 -target llvm-host-ir -entry computeMain -stage compute

//DIAGNOSTIC_TEST:SIMPLE(filecheck=REMOVED): -std 2026 -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=REMOVED): -std 2026 -target cpp -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=REMOVED): -std 2026 -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=REMOVED): -std 2026 -target llvm-host-ir -entry computeMain -stage compute

// Verify that the `&` (address-of) operator emits a deprecation warning
// under language version 2025, and a "removed declaration" error under
// language version 2026 and later.

RWStructuredBuffer<float> buffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // DEPRECATED: warning[E31200]: use of deprecated declaration
    // DEPRECATED: the '&' operator for taking addresses is deprecated
    // DEPRECATED-NOT: error[E31207]

    // REMOVED: error[E31207]: use of removed declaration
    // REMOVED: the '&' operator for taking addresses is no longer supported in Slang
    float* p = &buffer[threadId.x];
}
