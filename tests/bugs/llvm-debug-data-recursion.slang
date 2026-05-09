// This test just needs to run without crashing.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -llvm -compute -Xslang -O0 -Xslang -g3
//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct SelfReferential
{
    // There used to be a bug in the debug data emission here, where this would
    // cause infinite recursion inside the LLVM emitter.
    SelfReferential* self;
}

[numthreads(1,1,1)]
void computeMain(uint2 tid : SV_DispatchThreadID)
{
    SelfReferential self;
    self.self = &self;
    // CHECK: 0
    outputBuffer[0] = 0;
}
