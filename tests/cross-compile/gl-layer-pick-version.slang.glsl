//TEST_IGNORE_FILE:
#version 430

struct VS_OUT
{
	uint layerID;
};

vec4 main_(VS_OUT vsOut) : SV_Target
{
	return vec4(float(vsOut.layerID));
}

out vec4 SLANG_out_main_result;

void main()
{
	VS_OUT vsOut;
	vsOut.layerID = gl_Layer;

	vec4 main_result;
	main_result = main_(vsOut);

	SLANG_out_main_result = main_result;
}