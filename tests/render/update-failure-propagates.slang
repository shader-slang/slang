//TEST:COMPARE_COMPUTE_EX(filecheck=CHECK):-cpu -compute -shaderobj

// render-test must report a failure from `RenderTestApp::update` (here, `applyBinding` failing
// because the shader has no parameter named `doesNotExist`) through its result code, rather than
// exiting successfully without producing any output.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=doesNotExist
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = 1;
}

//CHECK: result code = 1
//CHECK: could not find shader parameter matching 'doesNotExist'
