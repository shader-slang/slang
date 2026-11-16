//TEST:SIMPLE(filecheck=MISSING,exit-code=-1): -target cubin -entry computeMain
//TEST:SIMPLE(filecheck=VIRTUAL,exit-code=-1): -target cubin -entry computeMain -Xnvrtc -arch=compute_80
//TEST:SIMPLE(filecheck=VIRTUAL,exit-code=-1): -target cubin -entry computeMain -Xnvrtc --gpu-architecture -Xnvrtc compute_80

// MISSING: CUBIN output requires an explicit CUDA SM capability
// VIRTUAL: CUBIN output requires a real architecture

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}
