//TEST:CROSS_COMPILE: -profile vs_5_0 -entry main -target spirv-assembly

struct VS_OUT
{
	float4 left : SV_Position;
    float4 right : NV_X_RIGHT;
    uint4  mask : NV_VIEWPORT_MASK;
};

VS_OUT main(float4 ll, float4 rr)
{
	VS_OUT res;
	res.left = ll;
	res.right = rr;
	res.mask = 0x1;
	return res;	
}