
// Emit 0xFFFF as the stride value for the unsized array

//TEST:SIMPLE(filecheck=CHECK): -entry fragment -stage fragment -emit-spirv-directly -target spirv-assembly -emit-spirv-directly
struct Descriptors {
  uint count;
  uint array[];
}

struct Context {
  Descriptors *descriptors;
}

[[vk::binding(0)]] ConstantBuffer<Context> context;

// Dummy entry point.
[shader("fragment")]
float4 fragment(): SV_Target
{
  return float4(float(context.descriptors[0].array[0]), 1., 1., 1.);
}

// CHECK: OpDecorate %_ptr_PhysicalStorageBuffer__runtimearr_uint ArrayStride 65535
