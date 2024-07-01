Texture2DMS <int3> t1;
Texture2DMSArray <float4> t2;

float4 main()
{
    uint2 p1 = uint2(1, 2);
    int3 a1 = t1.sample[7][p1];

    uint p2 = uint(1);
    float4 a2 = t2.sample[p2][uint3(1, 2, 3)];

    return float4(float3(a1), 0) + a2;
}
