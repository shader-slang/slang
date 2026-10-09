// shader-slang/slang#13530: a generic element type `T` in `uniform T values[]` cannot be
// classified before specialization, so specializing it with a resource type stays valid.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -specialize Texture2D

// CHECK-NOT: E31215
// CHECK: OpTypeRuntimeArray

struct Box<T>
{
    T value;
}

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main<T : IOpaqueDescriptor>(uniform T values[], uniform Box<T> boxes[])
{
    output[0] = 1;
}
