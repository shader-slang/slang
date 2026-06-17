//TEST:SIMPLE(filecheck=CHECK): -target spirv -O0

// The mip level reaches `OpImageFetch` through `__TextureMip__init`'s return
// value (a composite-extract of the boxed mip field), not as the literal
// constant directly, so the constant is pinned at the call that carries it.
// CHECK: OpFunctionCall %__TextureMip %__TextureMip__init %int_1
// CHECK: OpImageFetch %v4float {{.*}} Lod %{{[A-Za-z0-9_]+}}

RWStructuredBuffer<float4> result;

uniform Texture2D gTex;

[numthreads(1,1,1)]
void main()
{
    result[0] = gTex.mips[1][int2(3,4)];
}
