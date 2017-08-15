//TEST:COMPARE_HLSL: -no-checking -target dxbc-assembly -profile ps_4_0 -entry main

#ifdef __SLANG__
__import gh_160;
#endif

vec4 main(VS_OUT vOut) : SV_TARGET
{
	float3 color = float3(1,0,0);

    vec4 finalColor = vec4(color, 1.f);
    return finalColor;
}