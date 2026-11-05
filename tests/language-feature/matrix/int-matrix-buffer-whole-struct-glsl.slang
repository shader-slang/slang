//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -emit-spirv-via-glsl -stage compute -entry computeMain

// Loading a whole struct that has an int matrix member makes the buffer lowering copy the storage
// type into the logical type. GLSL has no `OpCopyLogical`, so this copy must be lowered into
// element-wise copies, and the compiler must not fail with an unexpected IR opcode.

struct S
{
    column_major int2x3 m;
    int tail;
}

RWStructuredBuffer<S> b;
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    let s = b[0];
    output[0] = s.tail + s.m[1][2];
}

// GLSL: void main()
// SPIRV: OpEntryPoint
