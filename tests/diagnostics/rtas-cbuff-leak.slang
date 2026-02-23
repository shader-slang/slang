//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry computeMain -target spirv -stage compute -preserve-params

// CHECK:rtas-cbuff-leak.slang(8): warning 31106
// CHECK:rtas-cbuff-leak.slang(10): warning 31107

// RTAS can't be a part of this binding, and will leak to a different binding.
[vk::push_constant]
cbuffer PushConstants {
    int i;
    RaytracingAccelerationStructure rtas;
};

[numthreads(1,1,1)]
void computeMain()
{
}
