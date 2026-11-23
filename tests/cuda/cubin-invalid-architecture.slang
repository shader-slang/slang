//TEST:SIMPLE(filecheck=MISSING,exit-code=2): -target cubin -entry computeMain
//TEST:SIMPLE(filecheck=VIRTUAL,exit-code=2): -target cubin -entry computeMain -Xnvrtc -arch=compute_80
//TEST:SIMPLE(filecheck=VIRTUAL,exit-code=2): -target cubin -entry computeMain -Xnvrtc --gpu-architecture -Xnvrtc compute_80
//TEST:SIMPLE(filecheck=MISSING,exit-code=2): -target cubin -entry computeMain -capability atomic_reduce
//TEST:SIMPLE(filecheck=LOW,exit-code=2): -target cubin -entry halfMain -capability cuda_sm_5_0
//TEST:SIMPLE(filecheck=LOW,exit-code=2): -target cubin -entry halfMain -capability cuda_sm_8_0 -Xnvrtc -arch=sm_50
//TEST:SIMPLE(filecheck=LOW,exit-code=2): -target cubin -entry halfMain -Xnvrtc -arch=compute_80 -Xnvrtc --gpu-architecture -Xnvrtc sm_50
//TEST:SIMPLE(filecheck=VIRTUAL,exit-code=2): -target cubin -entry computeMain -Xnvrtc -arch=sm_80 -Xnvrtc -arch=compute_80

// LOW: CUBIN target sm_50 is below the shader's minimum sm_60
// MISSING: CUBIN output requires an explicit CUDA SM capability
// VIRTUAL: CUBIN output requires a real architecture

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}

RWStructuredBuffer<half> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void halfMain()
{
    result[0] += half(1.0);
}
