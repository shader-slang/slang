//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -emit-spirv-directly

// SPIRV-NOT: OpDecorate {{.*}} ArrayStride 8
layout(binding = 0)
Texture2D textures[];

RWTexture2D<uint> tex;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    tex[threadId.xy] = uint(textures[0][threadId.xy].r);
}