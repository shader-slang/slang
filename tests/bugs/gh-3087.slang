//TEST:SIMPLE(filecheck=HLSL): -allow-glsl -entry main -target hlsl
//TEST:SIMPLE(filecheck=GLSL): -allow-glsl -entry main -target glsl
//TEST:SIMPLE(filecheck=SPIRV): -allow-glsl -entry main -target spirv

// HLSL-DAG: main
// HLSL-DAG: SV_InstanceID

// GLSL-DAG: main
// GLSL-DAG: gl_InstanceIndex

// SPIRV: OpEntryPoint
// SPIRV-NOT: BuiltIn InstanceIndex

struct PSInput
{
    uint vInstance : SV_InstanceID;
	float4 color : COLOR;
};

[shader("pixel")]
float4 main(PSInput input) : SV_TARGET
{
	return input.color + float(input.vInstance).xxxx;
}