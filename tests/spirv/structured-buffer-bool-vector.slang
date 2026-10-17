//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOBOOL):-target spirv -entry computeMain -stage compute

// A structured buffer whose element type is a bool vector stores it as an int vector,
// because SPIR-V forbids `OpTypeBool` in the StorageBuffer storage class
// (shader-slang/slang#13590).

StructuredBuffer<bool2> input;
RWStructuredBuffer<bool3> output;

// CHECK-DAG: OpTypeRuntimeArray %v2int
// CHECK-DAG: OpTypeRuntimeArray %v3int
// NOBOOL-NOT: OpTypeRuntimeArray %v{{[234]}}bool

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    bool2 v = input[tid.x];
    output[tid.x] = bool3(v.x, !v.y, v.x && v.y);
}
