// Test that texture subscripting in CUDA targets is properly diagnosed with the correct error message.
//TEST:SIMPLE(filecheck=CHECK): -target cuda

Texture1D<float> tex1D;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    
    float value = tex1D[idx];
}
// CHECK: error 55206: texture subscript operator is not supported for CUDA targets.
