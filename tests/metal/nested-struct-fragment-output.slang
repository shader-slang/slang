//TEST:SIMPLE(filecheck=METAL): -target metallib -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage fragment -entry fragmentMain

//METAL-DAG: color(0)
//METAL-DAG: color(1)
//METAL-DAG: color(2)
//METALLIB: @fragmentMain

RWStructuredBuffer<float> outputBuffer;

struct NestedFragment
{
    float2 p3;
}
struct Fragment
{
	float4 p1;
    float3 p2;
    NestedFragment p3;
};

struct FragmentStageInput
{
	float4	coarseVertex	: CoarseVertex;
};

struct FragmentStageOutput
{
	Fragment fragment	: SV_Target;
};

FragmentStageOutput fragmentMain(FragmentStageInput input)
{
    FragmentStageOutput output;
    output.fragment.p1 = float4(1.0f);
    output.fragment.p2 = float3(1.0f);
    output.fragment.p3.p3 = float2(1.0f);
	outputBuffer[0] = 1;
	return output;
}
