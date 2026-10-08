// Metal cannot store a texture in the `thread` address space. This program does not require such
// storage. Output specialization rewrites `selectTexture` so that its call produces `inputTexture`
// directly. SSA simplification can then remove the entry-point local before Metal emission.

//TEST:SIMPLE(filecheck=CHECK): -O0 -target metal -entry main -stage fragment -validate-ir -line-directive-mode none

// CHECK-NOT: thread texture
// CHECK-LABEL: fragment
// CHECK: texture2d<float, access::sample>
// CHECK-NOT: thread texture
// CHECK-NOT: selectedTexture

Texture2D<float4> inputTexture;
static Texture2D<float4> selectedTexture;

[noinline]
void selectTexture(out Texture2D<float4> result)
{
    result = inputTexture;
}

[shader("fragment")]
float4 main() : SV_Target
{
    selectTexture(selectedTexture);
    return selectedTexture.Load(int3(0, 0, 0));
}
