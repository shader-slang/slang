//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu

// CHECK: 28
// CHECK-NEXT: 0
// CHECK-NEXT: 28
// CHECK-NEXT: FFFFFFFC
// CHECK-NEXT: 28
// CHECK-NEXT: FFFFFFFC
// CHECK-NEXT: 29
// CHECK-NEXT: 18
// CHECK-NEXT: 29
// CHECK-NEXT: 18

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct SceneVertexEncoded
{
    float3 position;
    uint encodedNormal;
    uint encodedTangent;
    float2 texCoord;
};

void writeAddr(int i, uint64_t addr)
{
    outputBuffer[2*i] = uint(addr >> 32);
    outputBuffer[2*i+1] = uint(addr & 0xFFFFFFFF);
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    static const uint sizeofVertex = 3*4 + 1*4 + 1*4 + 2*4;
    static const uint vertexIndex = 0x9249249;

    SceneVertexEncoded* addr = reinterpret<SceneVertexEncoded*>(0x2800000000);
    writeAddr(0, uint64_t(addr));
    writeAddr(1, uint64_t(&addr[vertexIndex]));
    writeAddr(2, uint64_t(addr) + uint64_t(vertexIndex) * sizeofVertex);
    writeAddr(3, uint64_t(&addr[vertexIndex+1]));
    writeAddr(4, uint64_t(addr) + uint64_t(vertexIndex+1) * sizeofVertex);
}
