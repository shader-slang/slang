//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage pixel -entry fragmentMain

float4 fragmentMain(float2 uv : TEXCOORD) : SV_Target {
    float val = 1.5f;
    return float4(1.0f, ddx(val), 0.0f, 1.0f); 
}

//CHECK: return float4(1.0f, ddx(1.5f), 0.0f, 1.0f);
