//TEST:COMPARE_HLSL: -profile vs_5_0 -target dxbc-assembly

// Check handling of initializer list for vector

cbuffer C : register(b0)
{
	float4 a;	
};

float w0(float x) { return x; }
float w1(float x) { return x; }
float w2(float x) { return x; }
float w3(float x) { return x; }

float4 main() : SV_Position
{
    float4 wx = { w0(a.x), w1(a.x), w2(a.x), w3(a.x), };
    return wx;
}
