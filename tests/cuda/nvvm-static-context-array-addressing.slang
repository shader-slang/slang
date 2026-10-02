//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=indices
//TEST_INPUT:ubuffer(data=[91 0 0 0 0 0 0 92], stride=4):out,name=output
// CHECK: 91
// CHECK-NEXT: 203
// CHECK-NEXT: 17
// CHECK-NEXT: 31
// CHECK-NEXT: 214
// CHECK-NEXT: 28
// CHECK-NEXT: 42
// CHECK-NEXT: 92
struct Cell { int values[2]; };
static Cell state[2];
static int singleton[1];
StructuredBuffer<uint> indices;
RWStructuredBuffer<int> output;
[noinline] int writeState(uint i, int value)
{
    state[i].values[1] = value;
    singleton[0] += 3;
    return state[1 - i].values[0];
}
[shader("compute")]
[numthreads(2,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = indices[tid.x];
    int base = int(tid.x) * 11;
    state[0].values[0] = base + 17;
    state[1].values[0] = base + 17;
    singleton[0] = base + 28;
    int old = writeState(i, base + 203);
    output[1 + tid.x * 3] = state[i].values[1];
    output[2 + tid.x * 3] = old;
    output[3 + tid.x * 3] = singleton[0];
}
