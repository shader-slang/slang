//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute

// CHECK-DAG: @binding(0) @group(7)
// CHECK-DAG: @binding(3) @group(4)
// CHECK-DAG: @binding(1) @group(2)

[vk::binding(1, 2)]
Texture2D texA;

[vk::binding(3, 4)]
ConstantBuffer<float> cb;

RWStructuredBuffer<float> ob : register(u0, space7);

[numthreads(1,1,1)]
void computeMain()
{
    ob[0] = cb + texA.Load(int3(0)).x;
}