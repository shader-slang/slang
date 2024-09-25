//TEST:SIMPLE: -target wgsl

[format("rg8ui")]
Texture2D<uint> g_texture2D_rg8ui;

RWStructuredBuffer<float> output;

[numthreads(1,1,1)]
void computeMain()
{
    output[0] = g_texture2D_rg8ui.Load(int3(0, 0, 0)).x;
}