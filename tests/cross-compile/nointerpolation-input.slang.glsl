//TEST_IGNORE_FILE:

struct VS_OUT
{
	float drawID;
};

in flat float SLANG_in_vsOut_drawID;

out vec4 SLANG_out_main_result;

vec4 main_(VS_OUT vsOut)
{
	return vec4(vsOut.drawID);
}

void main()
{
	VS_OUT vsOut;
	vsOut.drawID = SLANG_in_vsOut_drawID;

	vec4 main_result;
	main_result = main_(vsOut);

	SLANG_out_main_result = main_result;
}
