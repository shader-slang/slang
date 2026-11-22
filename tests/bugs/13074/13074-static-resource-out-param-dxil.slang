//TEST:SIMPLE(filecheck=HLSL): -O0 -target hlsl -entry main -profile ps_6_6 -validate-ir
//TEST(dxc):SIMPLE(filecheck=DXIL): -O0 -target dxil -entry main -profile ps_6_6 -validate-ir

// A mutable `static` resource can be assigned from a non-uniform descriptor-heap index and then
// read by a separate helper. Resource-global replacement gives `loadSelectedTexture` a resource
// parameter and passes a load rooted at the entry-point local. That load is not one of the resource
// expression forms that HLSL can pass directly. The D3D resource-input step must inline the helper
// so that simplification can substitute the descriptor-heap conversion at the texture use.

// HLSL-NOT: loadSelectedTexture
// HLSL: ResourceDescriptorHeap[NonUniformResourceIndex
// HLSL-NOT: loadSelectedTexture
// DXIL: define void @main(

uniform uint textureIndex;
static Texture2D<float4> selectedTexture;

[noinline]
void selectTexture(out Texture2D<float4> result, uint index)
{
    result = ResourceDescriptorHeap[NonUniformResourceIndex(index)];
}

[noinline]
float4 loadSelectedTexture()
{
    return selectedTexture.Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    selectTexture(selectedTexture, textureIndex);
    return loadSelectedTexture();
}
