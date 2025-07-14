[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint3 result = uint3(1, 2, 3);
    int3 result2 = int3(4, 5, 6);
    float3 result3 = float3(1.0, 2.0, 3.0);
}
