// A module that uses DifferentialPair directly but never calls fwd_diff/bwd_diff still
// contains DifferentialPair IR, so the autodiff finalization passes must run to lower it.
// Guards that calcRequiredLoweringPassSet marks such modules as needing autodiff lowering
// (otherwise the pair type would survive un-lowered into emission).
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// Direct DifferentialPair use marks the module as needing autodiff lowering, so it
// takes the finalize path: finalizeAutoDiffPass runs to lower the pair type.
//TEST:SIMPLE(filecheck=PASS): -target hlsl -entry computeMain -stage compute -dump-ir-after finalizeAutoDiffPass
// PASS: ### AFTER finalizeAutoDiffPass:

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    DifferentialPair<float> dp = diffPair(3.0f, 1.0f);
    outputBuffer[0] = dp.p;
    outputBuffer[1] = dp.d;
    // CHECK: 3.0
    // CHECK: 1.0
}
