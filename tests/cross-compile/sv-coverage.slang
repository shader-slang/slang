// sv-coverage.slang

//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage fragment
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -emit-spirv-directly

float4 main(
	in float4 color : COLOR,
	in uint inputCoverage : SV_Coverage,
	out uint outputCoverage : SV_Coverage)
	: SV_Target
{
    // CHECK: %gl_SampleMask = OpVariable %_ptr_Input__arr_int_int_1 Input
	outputCoverage = inputCoverage ^ 1;
	return color;
}
