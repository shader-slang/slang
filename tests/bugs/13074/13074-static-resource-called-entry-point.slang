// An entry point can also be called as an ordinary function. Slang separates those two roles before
// moving the resource initializer. The shader entry point initializes its own resource value, while
// the ordinary clone receives the caller's current value without running that initializer again.
//
//TEST:SIMPLE(filecheck=CHECK): -O0 -target hlsl -validate-ir

Texture2D<float4> textures[2];
static Texture2D<float4> cachedTexture = textures[0];

[shader("fragment")]
[noinline]
float4 callee() : SV_Target
{
    return cachedTexture.Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    cachedTexture = textures[1];
    return callee();
}

// CHECK-LABEL: float4 callee()
// CHECK: return textures_0[int(0)].Load
// CHECK-LABEL: float4 callee_0(Texture2D<float4 > cachedTexture_0)
// CHECK: return cachedTexture_0.Load
// CHECK-LABEL: float4 main()
// CHECK: return callee_0(textures_0[int(1)]);
