//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -profile spirv_1_6

// Verify that calling the stdlib `inverse` function does not spuriously
// require `spvImageGatherExtended`.  The SPIR-V path only needs the
// GLSL.std.450 `MatrixInverse` extended instruction, which is available
// on any SPIR-V version without additional capabilities.

// CHECK-NOT: spvImageGatherExtended
// CHECK-NOT: warning
// CHECK: computeMain

import glsl;

RWStructuredBuffer<float4x4> buf;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    buf[1] = inverse(buf[0]);
}
