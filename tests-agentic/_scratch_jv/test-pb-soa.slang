//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
struct SoA { float values[4]; int counts[4]; }
ParameterBlock<SoA> g_pb;
RWStructuredBuffer<float> g_out;
//CHECK: cbuffer
[numthreads(1,1,1)]
void main()
{
    g_out[0] = g_pb.values[0] + float(g_pb.counts[0]);
}
