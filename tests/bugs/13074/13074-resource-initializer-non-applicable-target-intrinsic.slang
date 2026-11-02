// A CUDA-only target intrinsic does not replace `identity`'s function body when compiling HLSL. The
// resource-initializer analysis must therefore inspect that body and accept this initializer, which
// only copies the resource value supplied as its argument.
//
//TEST:SIMPLE: -target hlsl -entry main -stage fragment -validate-ir -o 13074-resource-initializer-non-applicable-target-intrinsic.hlsl

Texture2D<float4> inputTexture;

[noinline]
__target_intrinsic(cuda, "$0")
Texture2D<float4> identity(Texture2D<float4> value)
{
    return value;
}

static Texture2D<float4> cachedTexture = identity(inputTexture);

[shader("fragment")]
float4 main() : SV_Target
{
    return cachedTexture.Load(int3(0, 0, 0));
}
