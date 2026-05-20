//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
RWStructuredBuffer<int> buf;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float p = 0.0f;
    float n = -0.0f;
    uint pb = asuint(p);
    uint nb = asuint(n);
    buf[0] = int(p == n);
    buf[1] = int(pb);
    buf[2] = int(nb >> 31);
}
