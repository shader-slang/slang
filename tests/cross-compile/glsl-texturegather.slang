//TEST:CROSS_COMPILE(filecheck=CHECK): -profile ps_5_0 -entry main -target glsl

// CHECK: textureGather(sampler2DShadow(t_0,sc_0), (loc_0), (3.0))
// CHECK: textureGatherOffset(sampler2DShadow(t_0,sc_0), (loc_0), (3.0), (off_0))
// CHECK: textureGatherOffsets(sampler2DShadow(t_0,sc_0), (loc_0), (3.0), ivec2[]((off_0), (off_0), (off_0), (off_0)))
// CHECK: textureGatherOffsets(sampler2DShadow(t_0,sc_0), (loc_0), (3.0), ivec2[]((ivec2(6, 6)), (ivec2(7, 7)), (ivec2(8, 8)), (ivec2(9, 9))))

Texture2D t;
SamplerState s;
SamplerComparisonState sc;
float4 main()
{
    const float2 loc = float2(1,2);
    const float cmp = 3;
    const int2 off = int2(4,5);
    return t.GatherCmp(sc, loc, cmp)
         + t.GatherCmp(sc, loc, cmp, off)
         + t.GatherCmp(sc, loc, cmp, off, off, off, off)
         + t.GatherCmpRed(sc, loc, cmp, int2(6,6), int2(7,7), int2(8,8), int2(9,9));
}
