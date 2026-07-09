//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry computeMain -target spirv -stage compute -preserve-params

// RTAS can't be a part of this binding, and will leak to a different binding.
//
// CHECK: warning[E31106]
// CHECK:rtas-cbuff-leak.slang:[[# @LINE+2]]:
[vk::push_constant]
cbuffer PushConstants {
    int i;
    // CHECK: warning[E31107]
    // CHECK:rtas-cbuff-leak.slang:[[# @LINE+1]]
    RaytracingAccelerationStructure rtas;
};

[numthreads(1,1,1)]
void computeMain()
{
}
