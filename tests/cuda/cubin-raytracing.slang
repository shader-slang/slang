//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target cubin -entry rayMain -capability cuda_sm_8_0

// CHECK: CUBIN output is not supported for the OptiX ray-tracing pipeline

[shader("raygeneration")]
void rayMain()
{
}
