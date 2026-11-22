// A load through `ImmutablePtr<T>` uses the read-only path (`__ldg`) on CUDA. When the loaded
// value is itself a pointer (a pointer element, or a pointer member of a struct), it is loaded
// through the 64-bit integer `__ldg` and cast back, since `__ldg` has no pointer overload (#14144).

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

// CHECK: float * [[LDG:[A-Za-z0-9_]+]](float * * {{.*}})
// CHECK: ulonglong [[BITS:[A-Za-z0-9_]+]] = __ldg((slang_bit_cast<ulonglong *>(
// CHECK: return (slang_bit_cast<float *>([[BITS]]));
// CHECK: void computeMain(
// CHECK-DAG: = [[LDG]](&{{.*}}->data_0);
// CHECK-DAG: = [[LDG]](tables_0 +
// CHECK-DAG: = __ldg(&{{.*}}->scale_0);

// Three loads go through the read-only path: the two pointers and `scale`.
// PTX-COUNT-3: ld.global.nc

struct Node
{
    float* data;
    float scale;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform ImmutablePtr<Node> nodes,
    uniform ImmutablePtr<float*> tables,
    uniform float* result)
{
    float* d = nodes[tid.x].data;
    float* t = tables[tid.x];
    result[tid.x] = nodes[tid.x].scale * d[tid.x] + t[0];
}
