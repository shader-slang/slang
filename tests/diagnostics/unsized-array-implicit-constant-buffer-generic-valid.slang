// shader-slang/slang#13530: a generic element type `T` in `uniform T values[]` cannot be
// classified before specialization, so specializing it with a resource type stays valid.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -specialize Texture2D

// CHECK-NOT: E31215
// CHECK: %[[IMAGE:[A-Za-z0-9_]+]] = OpTypeImage %float 2D
// CHECK: OpTypeRuntimeArray %[[IMAGE]]

interface ILoadable
{
    float4 load(int i);
}

extension Texture2D : ILoadable
{
    float4 load(int i) { return Load(int3(i, 0, 0)); }
}

struct Box<T>
{
    T value;
}

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main<T : ILoadable>(uniform T values[], uniform Box<Box<T>> boxes[])
{
    output[0] = values[2].load(0) + boxes[1].value.value.load(1);
}
