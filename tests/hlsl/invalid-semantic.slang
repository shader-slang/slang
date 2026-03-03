//TEST:SIMPLE(filecheck=CHECK): -target spirv

struct Test_t
{
    uint4 foo;
};

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
RWStructuredBuffer< Test_t > g_Test : register(t0, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
ConstantBuffer< Test_t > g_Test2 : register(t1, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
StructuredBuffer< Test_t > g_Test3 : register(u2, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
ByteAddressBuffer g_Test4 : register(u3, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
RWTexture2D g_Test5 : register(t4, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
Texture2D g_Test6 : register(u5, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
FeedbackTexture2D g_Test7 : register(t6, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
AppendStructuredBuffer< Test_t > g_Test8 : register(t7, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
ConsumeStructuredBuffer< Test_t > g_Test9 : register(t8, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
RWByteAddressBuffer g_Test10 : register(t9, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
RasterizerOrderedByteAddressBuffer g_Test11 : register(t10, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
SamplerState g_Test12 : register(t11, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
SamplerComparisonState g_Test13 : register(t12, space1);

// CHECK: ([[# @LINE+1]]): error 31222: invalid HLSL register name
RaytracingAccelerationStructure g_Test14 : register(u13, space1);

[numthreads(1,1,1)]
void computeMain()
{
    Test_t test;
    test.foo = uint4( 0, 0, 0, 0 );
    g_Test[0] = test;
}