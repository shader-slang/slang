// precise-keyword.slang.hlsl
//TEST_IGNORE_FILE:

float4 main(float2 v_0 : V) : SV_TARGET
{
    precise float z_0;

    if(v_0.x > (float) 0)
    {
        z_0 = v_0.x * v_0.y + v_0.x;
    }
    else
    {
        z_0 = v_0.y * v_0.x + v_0.y;
    }

    return (float4) z_0;
}