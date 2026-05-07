// vector-invalid-swizzle.slang

// Test that invalid swizzle patterns are rejected.
// Note: Slang allows mixing xyzw and rgba sets (unlike GLSL), so we test
// actually invalid patterns: stpq (not supported) and non-member names.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    float4 v = float4(1.0, 2.0, 3.0, 4.0);

    float2 bad1 = v.st;
//CHECK:            ^^ member not found
//CHECK:            ^^ 'st' is not a member of 'vector<float,4>'.

    float bad2 = v.q;
//CHECK:           ^ member not found
//CHECK:           ^ 'q' is not a member of 'vector<float,4>'.
}
