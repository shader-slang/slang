//TEST:SIMPLE(filecheck=CHECK):-target glsl -profile glsl_450 -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv -profile glsl_450 -entry main -stage compute

// SPV: EntryPoint GLCompute {{.*}} "main" {{.*}}

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
    MyStruct s = gGlobals.pStruct.get();
    outputBuffer[tid.x] = uint(s.position.x);
}