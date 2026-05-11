// Regression coverage for textual backends referencing synthesized return-value
// storage outside of its declaring scope.
// https://github.com/shader-slang/slang/issues/11102

//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):  -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):  -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):   -target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):  -target wgsl -entry computeMain -stage compute

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-d3d12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cpu -compute -shaderobj -output-using-type
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Vec
{
    float data[5];

    [ForceInline]
    __init(float[5] values)
    {
        data = values;
    }

}

[ForceInline]
Vec make()
{
    Vec output;

    for (int mg = 0; mg < 1; mg++)
    {
        output.data[0] = 30.0f;
        output.data[1] = 70.0f;
    }

    return Vec(output.data);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint tid : SV_GroupIndex)
{
    let output = make();

    bool passed = output.data[0] == 30.0f && output.data[1] == 70.0f;
    if (tid == 0)
        outputBuffer[0] = passed ? 1 : 0;
}

// BUFFER: 1

// METAL: thread Vec_0 [[RET:_S[0-9]+]];
// METAL: for(;;)
// METAL: (&[[RET]])->data_0 = (&output_0)->data_0;
// METAL: if(((&[[RET]])->data_0[int(0)]) == 30.0)
// METAL: passed_0 = ((&[[RET]])->data_0[int(1)]) == 70.0;

// HLSL: Vec_0 [[RET:_S[0-9]+]];
// HLSL: for(;;)
// HLSL: [[RET]].data_0 = output_0.data_0;
// HLSL: if(([[RET]].data_0[int(0)]) == 30.0f)
// HLSL: passed_0 = ([[RET]].data_0[int(1)]) == 70.0f;

// CUDA: Vec_0 [[RET:_S[0-9]+]];
// CUDA: for(;;)
// CUDA: (&[[RET]])->data_0 = (&output_0)->data_0;
// CUDA: if(((&[[RET]])->data_0[int(0)]) == 30.0f)
// CUDA: passed_0 = ((&[[RET]])->data_0[int(1)]) == 70.0f;

// CPP: Vec_0 [[RET:_S[0-9]+]];
// CPP: for(;;)
// CPP: (&[[RET]])->data_0 = (&output_0)->data_0;
// CPP: if(((&[[RET]])->data_0[int(0)]) == 30.0f)
// CPP: passed_0 = ((&[[RET]])->data_0[int(1)]) == 70.0f;

// WGSL: var [[RET:_S[0-9]+]] : Vec_0;
// WGSL: for(;;)
// WGSL: [[RET]].data_0 = output_0.data_0;
// WGSL: if(([[RET]].data_0[i32(0)]) == 30.0f)
// WGSL: passed_0 = ([[RET]].data_0[i32(1)]) == 70.0f;
