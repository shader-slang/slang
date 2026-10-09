// Reflection must distinguish a zero-sized array (element count 0) from an
// unsized array (element count SLANG_UNBOUNDED_SIZE, printed as "unbounded").

//TEST:REFLECTION(filecheck=CHECK):-stage compute -entry main -target spirv -no-codegen

Texture2D<float4> zero_sized_arr[0];
Texture2D<float4> unsized_arr[];

RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = unsized_arr[2277].Load(int3(0, 0, 0));
}

// CHECK: "name": "zero_sized_arr",
// CHECK: "kind": "array",
// CHECK-NEXT: "elementCount": 0,
// CHECK: "name": "unsized_arr",
// CHECK: "kind": "array",
// CHECK-NEXT: "elementCount": "unbounded",
