//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -profile vs_5_0
interface IField
{
	float get();
};
struct GIn<TField>
{
	float3 p0;
	TField field;
};
struct F : IField
{
	float4 v0;
	float2 v1;
	float get() { return v0.x + v1.x; }
};

struct VOut
{
	float4 projPos : SV_POSITION;
};

VOut main(GIn<F> vIn)
{
	VOut vout;
	vout.projPos = float4(vIn.p0, vIn.field.get());
	return vout;
}