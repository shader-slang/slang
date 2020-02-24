// sv-coverage.slang

//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage fragment

float4 main(
	in float4 color : COLOR,
	in uint inputCoverage : SV_Coverage,
	out uint outputCoverage : SV_Coverage)
	: SV_Target
{
	outputCoverage = inputCoverage ^ 1;
	return color;
}
