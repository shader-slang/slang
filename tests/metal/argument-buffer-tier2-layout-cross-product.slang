//TEST:REFLECTION(filecheck=CHECK): -target metal -stage compute -entry main

// A ParameterBlock<Outer> that exercises the cross product of field kinds inside a Metal argument
// buffer, including an array of resource objects. Reflection reports each field in tier 2 bytes
// and tier 1 argument buffer slots. A field lists its two units in either order, so each pair uses
// CHECK-DAG.

struct Inner
{
    float a;
    float3 b;
    Texture2D t;
}

struct Outer
{
    Inner s;             // a struct of mixed types
    float scalarArr[3];  // an array of scalars
    float3 vecArr[2];    // an array of vectors
    float4x4 matArr[2];  // an array of matrices
    float arrArr[2][3];  // an array of arrays
    Inner structArr[2];  // an array of structs
    Texture2D texArr[2]; // an array of resource objects
}

ParameterBlock<Outer> block;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = block.s.a;
}

// Inner mixes a scalar at byte 0, a vector at byte 16, and a texture at byte 32, each with a slot.
// CHECK: "name": "s"
// CHECK: "name": "a"
// CHECK-DAG: {"kind": "uniform", "offset": 0, "size": 4, "elementStride": 0}
// CHECK-DAG: {"kind": "metalArgumentBufferElement", "index": 0}
// CHECK: "name": "b"
// CHECK-DAG: {"kind": "uniform", "offset": 16, "size": 16, "elementStride": 4}
// CHECK-DAG: {"kind": "metalArgumentBufferElement", "index": 1}
// CHECK: "name": "t"
// CHECK-DAG: {"kind": "uniform", "offset": 32, "size": 8, "elementStride": 0}
// CHECK-DAG: {"kind": "metalArgumentBufferElement", "index": 2}
// The struct as a whole occupies 48 bytes and three slots.
// CHECK-DAG: {"kind": "metalArgumentBufferElement", "index": 0, "count": 3}
// CHECK-DAG: {"kind": "uniform", "offset": 0, "size": 48, "elementStride": 0}

// An array of three scalars, one slot each, following the struct's three slots.
// CHECK: "name": "scalarArr"
// CHECK-DAG: {"kind": "uniform", "offset": 48, "size": 12, "elementStride": 4}
// CHECK-DAG: {"kind": "metalArgumentBufferElement", "index": 3, "count": 3}

// An array of two vectors, one slot each.
// CHECK: "name": "vecArr"
// CHECK-DAG: {"kind": "uniform", "offset": 64, "size": 32, "elementStride": 16}
// CHECK-DAG: {"kind": "metalArgumentBufferElement", "index": 6, "count": 2}

// An array of two matrices, four slots each.
// CHECK: "name": "matArr"
// CHECK-DAG: {"kind": "uniform", "offset": 96, "size": 128, "elementStride": 64}
// CHECK-DAG: {"kind": "metalArgumentBufferElement", "index": 8, "count": 8}

// An array of two inner arrays of three scalars, one slot each.
// CHECK: "name": "arrArr"
// CHECK-DAG: {"kind": "uniform", "offset": 224, "size": 24, "elementStride": 12}
// CHECK-DAG: {"kind": "metalArgumentBufferElement", "index": 16, "count": 6}

// An array of two Inner structs, three slots each.
// CHECK: "name": "structArr"
// CHECK-DAG: {"kind": "uniform", "offset": 256, "size": 96, "elementStride": 48}
// CHECK-DAG: {"kind": "metalArgumentBufferElement", "index": 22, "count": 6}

// An array of two textures, one slot each, with the bytes and the slots both scaled by the count.
// CHECK: "name": "texArr"
// CHECK-DAG: {"kind": "uniform", "offset": 352, "size": 16, "elementStride": 8}
// CHECK-DAG: {"kind": "metalArgumentBufferElement", "index": 28, "count": 2}
