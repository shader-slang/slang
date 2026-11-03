//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

// The module-wide analysis can prove this read uninitialized because no possible write to the
// global exists anywhere in the module. The post-link check sees the same no-write fact and must
// not issue a second diagnostic for the read of `selectedTexture`.

static Texture2D<float4> selectedTexture;

[shader("fragment")]
float4 main() : SV_Target
{
    return selectedTexture.Load(int3(0, 0, 0));
//CHECK:                       ^ use of uninitialized global variable
//CHECK:                       ^ use of uninitialized global variable 'selectedTexture'
}
