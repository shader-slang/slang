// Regression test for #13949: once `kEnabled` folds, the loop header's param for `x` has
// `undefined` from the entry edge and itself from the back edge. The peephole pass took the param
// as its own value, replaced its uses with itself and reported a change on every sweep, so
// compilation never finished.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -DINITIALIZED
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type -DINITIALIZED
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type -DINITIALIZED

// CHECK: OpEntryPoint

static const bool kEnabled = false;

//TEST_INPUT:ubuffer(data=[3 7], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
#ifdef INITIALIZED
    // The back edge passes the param itself and the entry edge passes `inputBuffer[1]`: the param
    // folds to that value.
    float x = float(inputBuffer[1]);
#else
    float x;
#endif
    for (uint i = 0; i < inputBuffer[0]; i++)
    {
        if (kEnabled)
            x = float(i);
    }
    outputBuffer[0] = x;
}

// BUF: type: float
// BUF-NEXT: 7.0
