//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry computeMain -stage compute -profile sm_5_0
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target glsl -emit-spirv-directly -entry computeMain -stage compute  -profile sm_5_0 -ignore-capabilities


// CHECK_IGNORE_CAPS-NOT: warning 41012

// CHECK: warning 41012
// CHECK-NOT: sm_6_0
// CHECK-NOT: sm_6_3
// CHECK-NOT: sm_6_4
// CHECK-SAME: vertex{{.*}}/{{.*}}fragment
// CHECK-SAME: sm_6_5

[numthreads(1,1,1)]
void computeMain()
{
    WaveMultiPrefixProduct(1, 0);
}
