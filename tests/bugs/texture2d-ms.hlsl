//TEST:CROSS_COMPILE: -profile lib_6_3 -entry main -stage compute -target spirv-assembly

[[vk::binding(0, 0)]]
Texture2DMS tex : register(t1);

[numthreads(4, 4, 1)]
void main(uint3 groupId : SV_GroupID)
{
    tex.Load(int2(groupId.xy), 0);
}
