//TEST:SIMPLE(filecheck=ERROR): -target spirv -entry computeMain -stage compute -DTEST_INTERNAL
//TEST:SIMPLE(filecheck=ERROR): -target spirv -entry computeMain -stage compute

// Test senario where a 'private'/'internal' member is not default-initialized
// for a struct but we call into a "public" memberwise ctor.
// The called "public" memberwise ctor should not be synth'ed.

// ERROR: error 30500
import dont_synth_public_constructor_helper;

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test val1 = { 1, { 2, 3 } };
	outputBuffer[0] = (true
        && val1.getVal1() == 2
        && val1.val2 == 2
        && val1.nested1.val1 == 2
        && val1.nested1.val2 == 2
        ) ? 1 : 0;
}