// Regression test for #13949: once `kEnabled` folds, the loop header's param for `x` has
// `undefined` from the entry edge and itself from the back edge. The peephole pass took the param
// as its own value, replaced its uses with itself and reported a change on every sweep, so
// compilation never finished.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=INIT): -target spirv -entry computeMain -stage compute -DINITIALIZED
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type -Xslang -DINITIALIZED
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type -Xslang -DINITIALIZED

// CHECK: OpEntryPoint

// With INITIALIZED, the param folds to `inputBuffer[1]`: the loop is gone and that value, converted
// to float, is stored with no OpPhi in between.
// INIT: %computeMain = OpFunction
// INIT-NOT: OpPhi
// INIT: %[[P:[0-9]+]] = OpAccessChain %{{[_a-zA-Z0-9]+}} %inputBuffer %int_0 %int_1
// INIT: %[[V:[0-9]+]] = OpLoad %uint %[[P]]
// INIT: %[[F:[0-9]+]] = OpConvertUToF %float %[[V]]
// INIT-NOT: OpPhi
// INIT: OpStore %{{[0-9]+}} %[[F]]
// INIT-NOT: OpPhi
// INIT: OpFunctionEnd

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
