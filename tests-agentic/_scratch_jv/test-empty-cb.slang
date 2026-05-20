//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
struct Single { float v; }
ConstantBuffer<Single> g_cb;
RWStructuredBuffer<float> g_out;
//CHECK: cbuffer
[numthreads(1, 1, 1)]
void main()
{
    g_out[0] = g_cb.v;
}
