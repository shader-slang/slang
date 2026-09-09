// Negative control for #12633: the E55215 rejection is scoped to *multisampled*
// textures, so a plain (non-multisampled) texture must still lower on CUDA. This
// guards the `isMultisample()` condition on the device-less CI path — without it,
// a regression that dropped or inverted that guard (rejecting all CUDA textures)
// would go uncaught, since the only other non-MS CUDA texture coverage
// (`cuda-texture.slang`) needs a CUDA device.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry main -stage compute

Texture2D<float> plainTexture;
// CUDA: CUtexObject
// CUDA-NOT: E55215

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = 1;
}
