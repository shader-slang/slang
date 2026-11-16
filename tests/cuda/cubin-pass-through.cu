//TEST:SIMPLE: -target cubin -pass-through nvrtc -entry computeMain -Xnvrtc -arch=sm_80 -o cubin-pass-through.cu.0.actual.cubin

extern "C" __global__ void computeMain()
{
}
