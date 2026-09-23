// Moving a resource initializer must not move an unrelated non-resource initializer. In particular,
// `getUnrelatedValue` may be called without first invoking this module's shader entry point.
//
//TEST:SIMPLE(filecheck=IR): -target hlsl -entry main -stage compute -g -validate-ir -verify-debug-serial-ir -dump-ir-after legalizeResourceGlobalVars -o 13074-resource-initializer-unrelated-state.hlsl

Texture2D<float4> texture;
RWStructuredBuffer<float> output;

[noinline]
Texture2D<float4> returnTexture(Texture2D<float4> value)
{
    return value;
}

static Texture2D<float4> cachedTexture = returnTexture(texture);
static uint unrelatedValue = 17;

// IR: {{^}}[fileOrNamespaceScopeStaticVar]
// IR: {{^}}[nameHint("unrelatedValue")]
// IR: {{^}}global_var %unrelatedValue
// IR: {{^}}{
// IR: {{^}}block
// IR: {{^[ \t]*}}return_val(17 : UInt)

export uint getUnrelatedValue()
{
    return unrelatedValue;
}

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0)).x;
}
