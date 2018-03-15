//TEST(compute):COMPARE_COMPUTE:

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):dxbinding(0),glbinding(0),out
//TEST_INPUT:ubuffer(data=[1 2 3 0], stride=4):dxbinding(1),glbinding(1)

// Check that we propagate the `[unroll]` attribute
// through to HLSL output correctly.
//
// If we neglect to generate the attribute in the output,
// it will generate a warning output from fxc, and the
// test will fail to match the expected output.

RWStructuredBuffer<int> buffers[2];

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	// Note: using `unroll` as a variable name to validate that
	// the lookup process for attribute names doesn't run into
	// problems because of local declarations with the same name.
	int unroll = buffers[1][tid];

	[unroll]
	for(int ii = 0; ii < 2; ii++)
	{
		unroll = buffers[ii][unroll];
	}

	buffers[0][tid] = unroll;
}