// Test that local matrices have correct debug info regardless of -matrix-layout-column-major flag
// Local matrices should always be row-major in SPIR-V, but the debug info was incorrectly
// showing them as column-major when the flag was set.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv -g3 -O0 -stage compute -entry main -matrix-layout-column-major

// This matrix should be column-major (uniform storage)
uniform float2x4 m;

RWStructuredBuffer<float4> outputData;

[numthreads(1, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    // This matrix should not be column-major (local storage)
    float2x4 m2x4 = float2x4(
        1.0, 2.0, 3.0, 4.0,  // row 0
        5.0, 6.0, 7.0, 8.0   // row 1
    );

    outputData[0] = (m2x4 + m)[0];
}

// Check that the DebugTypeMatrix for the local variable m2x4 has isColumnMajor = false (0)
// The debug type should show: DebugTypeMatrix with 2 vectors and false column-major flag

// SPIRV: OpExtInst %void {{.*}} DebugTypeMatrix {{.*}} %uint_2 %false
// SPIRV: OpExtInst %void {{.*}} DebugLocalVariable {{.*}} %{{[0-9]+}}
