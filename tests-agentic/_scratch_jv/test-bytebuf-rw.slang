//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
RWByteAddressBuffer g_buf;
RWStructuredBuffer<uint> g_out;
//CHECK: RWByteAddressBuffer
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    g_buf.Store(0, tid.x);
    g_out[0] = g_buf.Load(0);
}
