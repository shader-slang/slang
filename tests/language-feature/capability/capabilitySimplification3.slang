//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry computeMain -stage compute -profile sm_5_0
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target glsl -emit-spirv-directly -entry computeMain -stage compute  -profile sm_5_0 -ignore-capabilities


// CHECK_IGNORE_CAPS-NOT: error 36107

// CHECK: error 36107: entrypoint 'computeMain' does not support compilation target 'glsl' with stage 'compute'
// CHECK: capabilitySimplification3.slang(15): note: see using of 'WaveMultiPrefixProduct'
// CHECK-NOT: see using of 'WaveMultiPrefixProduct'
// CHECK: {{.*}}.meta.slang({{.*}}): note: see definition of 'WaveMultiPrefixProduct'

[numthreads(1,1,1)]
void computeMain()
{
    WaveMultiPrefixProduct(1, 0);
}
