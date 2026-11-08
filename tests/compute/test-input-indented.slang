// render-test reads `//TEST_INPUT:` lines that are indented, as slang-test does for `//TEST`.
// Both inputs below sit next to their entry-point parameters, one indented with spaces and one
// with a tab. Before the fix they were skipped, so the parameters were left unbound.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -cuda -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(
    //TEST_INPUT:ubuffer(data=[2.0], stride=4):name=a
    uniform StructuredBuffer<float> a,
	//TEST_INPUT:ubuffer(data=[3.0], stride=4):name=b
	uniform StructuredBuffer<float> b)
{
    outputBuffer[0] = a[0];
    outputBuffer[1] = b[0];
}

// BUF: 2.0
// BUF-NEXT: 3.0
