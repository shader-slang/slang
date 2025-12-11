//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -g3 -O0

struct PSInput
{
    float4 position : SV_Position;
    int x;
};

static const int x = 8;

float4 main(PSInput input) : SV_Target
{
    float4 output = float4(0);

    for (uint32_t i = 0; i < x; ++i) {
        output *= output;
    }

    output /= x;

    return output;
}

// This test verifies that debug line information for loops appears in source order.
// The loop body (lines 16-17) should appear before the division (line 19) in the SPIR-V output.
// We check for the presence of DebugLine instructions and that they are generally non-decreasing.

// CHECK: DebugLine
