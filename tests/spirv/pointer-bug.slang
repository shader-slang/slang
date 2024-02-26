//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly
struct Foo {
  float4 *positions;
};

struct Params {
  Foo *foo;
};

// CHECK: %_ptr_PhysicalStorageBuffer_Foo = OpTypePointer PhysicalStorageBuffer %Foo

[[vk::push_constant]] Params params;

[shader("compute")]
[numthreads(1,1,1)]
void main() {
  params.foo.positions[10] += float4(1, 1, 1, 1);
}