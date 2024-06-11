//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//CHECK_HLSL: u4, space2
//CHECK_HLSL: u5, space1
//CHECK_HLSL: u6
//CHECK_HLSL: u7
//CHECK_HLSL: u9
//CHECK_HLSL: u12

//CHECK_GLSL: binding = 4, set = 2
//CHECK_GLSL: binding = 5, set = 1
//CHECK_GLSL: binding = 6
//CHECK_GLSL: binding = 7
//CHECK_GLSL: binding = 10
//CHECK_GLSL: binding = 12

[[vk::binding(4,2)]]
RWStructuredBuffer<int> b0 : register(u4, space2);

RWStructuredBuffer<int> b1 : register(u5, space1);

RWStructuredBuffer<int> b2 : register(u6);

[[vk::binding(7,0)]]
RWStructuredBuffer<int> b3 : register(u7, space0);

[[vk::binding(10,0)]]
RWStructuredBuffer<int> b4[2] : register(u9);

RWStructuredBuffer<int> b5[2] : register(u12);

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
	int tid = dispatchThreadID.x;
	b0[0] = 1;
    
    b1[0] = 1;
    
    b2[0] = 1;
    
    b3[0] = 1;
    
    b4[0][0] = 1;
    b4[0][1] = 1;

    b5[0][1] = 1;
}
