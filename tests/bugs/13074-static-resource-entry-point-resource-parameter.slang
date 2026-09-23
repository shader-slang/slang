//TEST:SIMPLE: -target hlsl -entry main -stage fragment -g -validate-ir -verify-debug-serial-ir -o 13074-static-resource-entry-point-resource-parameter.hlsl
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment -validate-ir -verify-debug-serial-ir

// Entry-point legalization moves a resource parameter to module scope and records its source entry
// point with `IREntryPointParamDecoration`. That decoration is metadata, not another way to invoke
// `main`, so it must not prevent resource-global legalization.

static Texture2D<float4> cachedTexture;

[shader("fragment")]
float4 main(Texture2D<float4> inputTexture) : SV_Target
{
    cachedTexture = inputTexture;
    return cachedTexture.Load(int3(0, 0, 0));
}

//CHECK-LABEL: void main()
//CHECK: .Load
