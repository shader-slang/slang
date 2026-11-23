//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target cubin -entry rayMain -capability cuda_sm_8_0
//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target cubin -whole-program -entry rayMain -entry computeMain -capability cuda_sm_8_0
//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target cubin -whole-program -entry computeMain -entry rayMain -capability cuda_sm_8_0

// CHECK: CUBIN output is not supported for the OptiX ray-tracing pipeline

[shader("raygeneration")]
void rayMain()
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}
