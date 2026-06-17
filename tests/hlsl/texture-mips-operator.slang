//TEST:SIMPLE(filecheck=CHECK): -target spirv -O0

// CHECK: OpImageFetch %v4float {{.*}} Lod %{{[A-Za-z0-9_]+}}

RWStructuredBuffer<float4> result;

uniform Texture2D gTex;

[numthreads(1,1,1)]
void main()
{
    result[0] = gTex.mips[1][int2(3,4)];
}
