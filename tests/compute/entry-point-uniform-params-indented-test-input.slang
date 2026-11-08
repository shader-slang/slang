// Confirm that render-test honours a `//TEST_INPUT:` line that does not start
// in column 0, using the same prefix rule as slang-test's `//TEST` scan:
// leading spaces or tabs (the `b` line is indented with a tab on purpose),
// extra slashes after `//`, and spaces before `TEST_INPUT:` are all accepted.
// A commented-out `// //TEST_INPUT:` line is not a directive; if it were
// parsed, its `decoy` entry would name no shader parameter and render-test
// would fail.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -cuda -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -metal -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(
    //TEST_INPUT:ubuffer(data=[2.0], stride=4):name=a
    uniform StructuredBuffer<float> a,
	//TEST_INPUT:ubuffer(data=[3.0], stride=4):name=b
    uniform StructuredBuffer<float> b,
    /// TEST_INPUT:ubuffer(data=[4.0], stride=4):name=c
    // //TEST_INPUT:ubuffer(data=[9.0], stride=4):name=decoy
    uniform StructuredBuffer<float> c)
{
    outputBuffer[0] = a[0];
    outputBuffer[1] = b[0];
    outputBuffer[2] = c[0];
}

// BUF: 2.0
// BUF-NEXT: 3.0
// BUF-NEXT: 4.0
