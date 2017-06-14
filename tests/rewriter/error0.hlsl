//TEST(smoke):COMPARE_HLSL: -no-checking -target dxbc-assembly -profile ps_4_0 -entry main

// We need to confirm that when there is an error in
// the input code, we allow the downstream compiler
// to detect and report the error, not us...

// This file presents a simple case, where we forgot a semicolon.

float4 main() : SV_Target
{
    float a = 1.0;

    // no semicolon at the end of this line!
    float b = 2.0

    float c = a + b;

    return float4(c);
}