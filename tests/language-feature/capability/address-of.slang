//TEST:SIMPLE(filecheck=CHECK_FAIL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry computeMain -stage compute

// Test that __getAddress correctly reports capabilities.

uniform int* outputBuffer;
uniform int* buffer;

// CHECK_PASS: OpEntryPoint
// CHECK_PASS-NOT: error

// CHECK_FAIL: ([[# @LINE+1]]): error 36107{{.*}}glsl
void computeMain()
{
    // CHECK: ([[# @LINE+1]]): note: see using of '__getAddress'
    outputBuffer[0] = *(__getAddress(buffer[0]));
}