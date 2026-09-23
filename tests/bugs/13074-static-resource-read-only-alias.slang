//TEST:SIMPLE: -target hlsl -entry main -stage fragment -validate-ir -o 13074-static-resource-read-only-alias.hlsl
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment -validate-ir

// A read-only explicit argument may refer to the same resource that the callee also reads by name.
// The source parameter and generated parameter preserve the two reads without introducing
// competing writes.

//CHECK-LABEL: float4 readExplicitlyAndByName_
//CHECK: .Load({{.*}}) + {{.*}}.Load({{.*}});
//CHECK-LABEL: float4 main()
//CHECK: readExplicitlyAndByName_

Texture2D<float4> textures[1];
static Texture2D<float4> selectedTexture;

[noinline]
float4 readExplicitlyAndByName(__constref Texture2D<float4> explicitTexture)
{
    return explicitTexture.Load(int3(0, 0, 0)) + selectedTexture.Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    selectedTexture = textures[0];
    return readExplicitlyAndByName(selectedTexture);
}
