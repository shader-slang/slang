//TEST:SIMPLE: -target hlsl -entry main -stage compute -validate-ir -verify-debug-serial-ir -o 13074-static-resource-member.hlsl
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage compute -validate-ir -verify-debug-serial-ir

// A static data member and a file- or namespace-scope `static` variable both lower to IR global
// variables, but resource-global legalization handles only the file- or namespace-scope form. The
// emitted global declaration and assignment prove that resource-global legalization did not
// replace this member with an entry-point local.

//CHECK: static Texture2D<float4 > [[CACHE:cachedTexture_[0-9]+]];
//CHECK-LABEL: void main()
//CHECK: [[CACHE]] = sourceTexture_{{[0-9]+}};

struct ResourceCache
{
    static Texture2D<float4> cachedTexture;
}

Texture2D<float4> sourceTexture;
RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void main()
{
    ResourceCache.cachedTexture = sourceTexture;
    output[0] = ResourceCache.cachedTexture.Load(int3(0, 0, 0));
}
