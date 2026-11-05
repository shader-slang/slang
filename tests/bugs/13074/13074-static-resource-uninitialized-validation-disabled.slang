//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute -validate-ir -disable-non-essential-validations -warnings-as-errors all

// Resource-global replacement creates the entry-point local after the pre-link module-wide
// uninitialized-value check. Disabling non-essential validation must suppress both that check and
// the later generated-local analysis. Promoting every warning to an error makes the test fail if
// the later analysis reintroduces the possible-uninitialized-use warning.

Texture2D<float4> textures[1];
RWStructuredBuffer<float4> output;
uniform bool selectTexture;
static Texture2D<float4> selectedTexture;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    if (selectTexture)
        selectedTexture = textures[0];
    output[0] = selectedTexture.Load(int3(0, 0, 0));
}
