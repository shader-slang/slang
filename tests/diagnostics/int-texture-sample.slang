//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void test(Texture2D<int4> itex, SamplerState s)
{
    // CHECK: ([[# @LINE+1]]): error
    itex.Sample(s, float2(0.0, 0.0));
}
