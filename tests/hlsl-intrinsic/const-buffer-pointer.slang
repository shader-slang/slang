//TEST:SIMPLE(filecheck=CHECK):-target glsl -profile glsl_450 -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv -profile glsl_450 -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv -profile glsl_450 -entry main -stage compute -emit-spirv-directly
// SPV: OpMemoryModel PhysicalStorageBuffer64
// SPV: OpEntryPoint GLCompute {{.*}} "main" {{.*}}
// SPV: OpTypePointer PhysicalStorageBuffer
// SPV: OpINotEqual
// SPV: OpConvertPtrToU
// SPV: OpIAdd
// SPV: OpConvertUToPtr
// SPV: OpLoad
struct MyStruct
{
    float4 position;
    float4x4 transform;
}

// CHECK: layout(buffer_reference, std430, buffer_reference_align = 16) readonly buffer BufferPointer_MyStruct
// CHECK-NEXT: {
// CHECK-NEXT: MyStruct{{.*}} _data;
// CHECK-NEXT: }

// CHECK: struct Globals
// CHECK-NEXT: {
// CHECK-NEXT: BufferPointer_MyStruct{{.*}} pStruct
// CHECK-NEXT: }

// CHECK: void main
// CHECK: MyStruct{{.*}} s{{.*}} = ((gGlobals{{.*}}.pStruct{{.*}})._data);

struct Globals
{
    ConstBufferPointer<MyStruct> pStruct;
}

ConstantBuffer<Globals> gGlobals;

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1,1,1)]
void main(int3 tid: SV_DispatchThreadID)
{
    if (gGlobals.pStruct.isValid())
    {
        MyStruct s = gGlobals.pStruct.get();
        outputBuffer[tid.x] = uint(s.position.x);
        MyStruct s1 = gGlobals.pStruct[3];
        outputBuffer[tid.x + 1] = uint(s1.position.x);
    }
}
