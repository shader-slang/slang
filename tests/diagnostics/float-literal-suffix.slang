//TEST:SIMPLE:-target dxil -profile ps_6_0 -entry test:
// Test that float literals are correctly emitted with 'f' suffix in HLSL output
// to ensure compatibility with DXC without requiring -HV 202x flag.

float4 test(float2 uv : TEXCOORD) : SV_Target
{
    // Test various float literal cases
    float val = 1.5;                   // Basic float literal
    float val2 = 0.0;                  // Zero
    float val3 = -2.5;                 // Negative float
    float val4 = 1000.0;               // Large float
    float val5 = 0.000001;             // Small float
    
    // Test float literals in intrinsic function calls that only accept float
    float dx = ddx(1.5);               // ddx only accepts float, not double
    float dy = ddy(2.0);               // ddy only accepts float, not double
    float fwidth_val = fwidth(3.0);    // fwidth only accepts float, not double
    
    // Test float literals in vector construction
    float4 vec = float4(1.0, 2.0, 3.0, 4.0);
    
    // Return using float literals to ensure they're all properly suffixed
    return float4(dx, dy, fwidth_val, 1.0);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):
//TEST:CROSS_COMPILE: -profile ps_6_0 -entry test 