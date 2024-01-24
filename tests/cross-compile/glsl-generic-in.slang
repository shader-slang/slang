//TEST:SIMPLE(filecheck=CHECK): -target spirv-assembly -entry main -profile vs_5_0
// CHECK: vIn_field_v0_0 = OpVariable %_ptr_Input_v4float Input
// CHECK: %vIn_field_v1_0 = OpVariable %_ptr_Input_v2float Input
// CHECK: %vIn_p0_0 = OpVariable %_ptr_Input_v3float Input
interface IField
{
	float get();
};
struct GIn<TField : IField, TEmptyField>
{
	float3 p0;
	TField field;
	TEmptyField e;
};
struct F : IField
{
	float4 v0;
	float2 v1;
	float get() { return v0.x + v1.x; }
};
struct E
{
	float get() {return 1.0;}
};

struct VOut
{
	float4 projPos : SV_POSITION;
};

VOut main(GIn<F, E> vIn)
{
	VOut vout;
	vout.projPos = float4(vIn.p0, vIn.field.get() + vIn.e.get());
	return vout;
}
