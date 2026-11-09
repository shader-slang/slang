//TEST:SIMPLE(filecheck=CHECK): -target hgbc

[shader("compute")]
[numthreads(1, 1, 1)]
void copyFirstComponent(StructuredBuffer<half2> input, RWStructuredBuffer<float> output)
{
    output[0] = float(input[0].x);
}

[host]
public void main()
{
    StructuredBuffer<half2> input = StructuredBuffer<half2>::alloc(1);
    RWStructuredBuffer<float> output = RWStructuredBuffer<float>::alloc(1);
    dispatch(copyFirstComponent, uint3(1, 1, 1))(input, output);
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[HALF:%[A-Za-z0-9_]+]] = OpFloat16Type
// CHECK-DAG: [[HALF2:%[A-Za-z0-9_]+]] = OpVectorType 2 [[HALF]]
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpStructuredBufferType [[HALF2]]
// CHECK: {{%[A-Za-z0-9_]+}} : [[BUFFER]] = OpAllocateResource Buffer Buffer Undefined DeviceLocal UnorderedAccess
// CHECK-SAME: [[HALF2]]
