//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1): -target spirv -entry fsInoutInputOnly -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2): -target spirv -entry fsInoutBidirectional -stage fragment

// Test inout parameter semantics validation.
// Some semantics support both input and output (bidirectional), while others are input-only or output-only.

// Test 1: Using input-only semantic as inout should error
// SV_IsFrontFace is input-only in fragment shader
float4 fsInoutInputOnly(float4 pos : SV_Position, inout bool frontFace : SV_IsFrontFace) : SV_Target
//CHECK1:                                                    ^^^^^^^^^ system value semantic 'SV_IsFrontFace' cannot be used as output in 'pixel' shader stage
{
    frontFace = !frontFace;
    return pos;
}

// Test 2: Using bidirectional semantic as inout should work
// SV_Coverage supports both get and set in fragment shader
float4 fsInoutBidirectional(float4 pos : SV_Position, inout uint coverage : SV_Coverage) : SV_Target
{
    coverage = coverage & 0x1;
    return pos;
}
