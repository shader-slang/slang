//DIAGNOSTIC_TEST:SIMPLE: -target cuda -entry VSMain -stage vertex

// Regression test: CUDA target ICE on vertex/fragment varying parameters
// Issue: https://github.com/shader-slang/slang/issues/10241

float4 VSMain(float4 pos : POSITION) : SV_POSITION { return pos; }
