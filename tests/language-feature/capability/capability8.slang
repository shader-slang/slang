//TEST:SIMPLE(filecheck=CHECK-PASS): -target glsl -entry computeMain -stage compute -profile sm_5_0
//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute -profile sm_5_0

// Test that we diagnose an error when calling a method that requires a capability that is not
// available in the target.

RWStructuredBuffer<int> outputBuffer;

[require(cpp_cuda_glsl_hlsl_metal_spirv_wgsl, byteaddressbuffer)]
struct Test
{
    [require(cpp_cuda_glsl_hlsl_spirv_wgsl, structuredbuffer)]
    int data()
    {
        return 0;
    }
}

// CHECK-PASS-NOT: error 36107
// CHECK: ([[# @LINE+1]]): error 36107{{.*}}metal
void computeMain()
{
    Test t;
    // CHECK: ([[# @LINE+1]]): note: see using of 'data'
    outputBuffer[0] = t.data();
}