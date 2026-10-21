//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry main

// An enum-typed specialization constant in `[numthreads]` is emitted with the WGSL type of
// the enum's underlying type, so a `uint`-backed enum is converted to i32 in `@workgroup_size`
// and in the `WorkgroupSize()` vector. See shader-slang/slang#13767.

// CHECK-DAG: @id(3) override groupSizeX_0 : i32
// CHECK-DAG: @id(4) override groupSizeY_0 : u32
// CHECK: @workgroup_size(groupSizeX_0, i32(groupSizeY_0), 1)
// CHECK: writeSize_0(vec3<i32>(groupSizeX_0, i32(groupSizeY_0), i32(1)))

enum class SignedSize : int
{
    X = 8,
}

enum class UnsignedSize : uint
{
    Y = 4,
}

[vk::constant_id(3)]
const SignedSize groupSizeX = SignedSize::X;

[vk::constant_id(4)]
const UnsignedSize groupSizeY = UnsignedSize::Y;

RWStructuredBuffer<int> output;

[noinline]
void writeSize(int3 size)
{
    output[0] = size.x;
    output[1] = size.y;
    output[2] = size.z;
}

[numthreads(groupSizeX, groupSizeY, 1)]
void main()
{
    writeSize(WorkgroupSize());
}
