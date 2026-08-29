//TEST(compute):COMPARE_COMPUTE: -shaderobj
//TEST(compute):COMPARE_COMPUTE: -cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE: -cuda -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm
//TEST:SIMPLE(filecheck=NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o -

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4),name=buf
RWStructuredBuffer<float4> buf;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    float sum = 0.0f;
    
    buf[0].wxyz = float4(1.0, 0.0, 0.0, 0.0);
    sum += buf[0].w;
    
    buf[0].xwyz = float4(0.0, 1.0, 0.0, 0.0);
    sum += buf[0].w;

    buf[0].xywz = float4(0.0, 0.0, 1.0, 0.0);
    sum += buf[0].w;

    buf[0].xyzw = float4(0.0, 0.0, 0.0, 1.0);
    sum += buf[0].w;

    outputBuffer[0] = int(sum);
}

// NVVM: st.global.u32
// NVVM: ld.global.v4.f32
// NVVM: cvt.rzi.s32.f32
