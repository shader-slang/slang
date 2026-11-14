// D3D accepts only a struct as a hit attribute, both as ReportHit's argument and as a closesthit
// or anyhit attribute parameter. A non-struct attribute gets a one-field wrapper struct, built by
// value at ReportHit and read through its data field in the receivers. The wrapper has its value
// type's layout, so a sender and a receiver compiled separately still agree. User struct
// attributes and the Khronos targets are unchanged.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile lib_6_6 -whole-program -validate-ir-detailed
//TEST:SIMPLE(filecheck=PAQ): -target hlsl -profile lib_6_7 -whole-program -validate-ir-detailed
//TEST:SIMPLE(filecheck=DXIL): -target dxil-asm -profile lib_6_6 -whole-program
//TEST:SIMPLE(filecheck=DXIL): -target dxil-asm -profile lib_6_7 -whole-program
//TEST:SIMPLE(filecheck=SEND): -target hlsl -profile lib_6_6 -entry isectVector -stage intersection
//TEST:SIMPLE(filecheck=RECV): -target hlsl -profile lib_6_6 -entry closestHitVector -stage closesthit
//TEST:SIMPLE(filecheck=RECV): -target hlsl -profile lib_6_6 -entry anyHitVector -stage anyhit
//TEST:SIMPLE(filecheck=SOLOSEND): -target dxil-asm -profile lib_6_6 -entry isectVector -stage intersection
//TEST:SIMPLE(filecheck=SOLORECV): -target dxil-asm -profile lib_6_6 -entry closestHitVector -stage closesthit
//TEST:SIMPLE(filecheck=SOLORECV): -target dxil-asm -profile lib_6_6 -entry anyHitVector -stage anyhit
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry isectVector -stage intersection -emit-spirv-directly -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry closestHitVector -stage closesthit -emit-spirv-directly -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry anyHitVector -stage anyhit -emit-spirv-directly -validate-ir

struct UserAttributes
{
    float2 coords;
}

RWStructuredBuffer<float> result;

[shader("intersection")]
void isectVector()
{
    ReportHit(1.0, 0, float2(0.25, 0.5));
}

[shader("intersection")]
void isectScalar()
{
    ReportHit(2.0, 1, result[0]);
}

[shader("intersection")]
void isectArray()
{
    float values[2] = { result[1], result[2] };
    ReportHit(3.0, 2, values);
}

[shader("intersection")]
void isectStruct()
{
    UserAttributes attributes;
    attributes.coords = float2(result[3], result[4]);
    ReportHit(4.0, 3, attributes);
}

[shader("closesthit")]
void closestHitVector(inout float4 payload, in float2 attributes)
{
    payload = float4(attributes, 0.0, 1.0);
}

[shader("anyhit")]
void anyHitVector(inout float4 payload, float2 attributes)
{
    if (attributes.x > 0.5)
        IgnoreHit();
    payload.x = attributes.y;
}

[shader("closesthit")]
void closestHitScalar(inout float4 payload, in float attributes)
{
    payload.x = attributes;
}

[shader("closesthit")]
void closestHitArray(inout float4 payload, in float attributes[2])
{
    payload.xy = float2(attributes[0], attributes[1]);
}

[shader("closesthit")]
void closestHitStruct(inout float4 payload, in UserAttributes attributes)
{
    payload.xy = attributes.coords;
}

// HLSL: struct [[userType:UserAttributes[A-Za-z0-9_]*]]
// HLSL: struct [[arrayType:HitAttributes_t[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: float [[arrayField:[A-Za-z0-9_]+]][int(2)];
// HLSL: struct [[scalarType:HitAttributes_t[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: float [[scalarField:[A-Za-z0-9_]+]];
// HLSL: struct [[vectorType:HitAttributes_t[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: float2 [[vectorField:[A-Za-z0-9_]+]];
// HLSL: void isectVector()
// HLSL: [[vectorType]] [[vectorArg:[A-Za-z0-9_]+]] = { float2(0.25f, 0.5f) };
// HLSL: ReportHit((1.0f), (0U), ([[vectorArg]]))
// HLSL: void isectScalar()
// HLSL: [[scalarType]] [[scalarArg:[A-Za-z0-9_]+]] = { result{{[A-Za-z0-9_]*}}[int(0)] };
// HLSL: ReportHit((2.0f), (1U), ([[scalarArg]]))
// HLSL: void isectArray()
// HLSL: [[arrayType]] [[arrayArg:[A-Za-z0-9_]+]] = { values{{[A-Za-z0-9_]*}} };
// HLSL: ReportHit((3.0f), (2U), ([[arrayArg]]))
// HLSL: void isectStruct()
// HLSL: [[userType]] [[userArg:[A-Za-z0-9_]+]];
// HLSL: ReportHit((4.0f), (3U), ([[userArg]]))
// HLSL: void closestHitVector(inout RayPayload_t{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}}, [[vectorType]] [[chVector:[A-Za-z0-9_]+]])
// HLSL: = float4([[chVector]].[[vectorField]], 0.0f, 1.0f);
// HLSL: void anyHitVector(inout RayPayload_t{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}}, [[vectorType]] [[ahVector:[A-Za-z0-9_]+]])
// HLSL: if(([[ahVector]].[[vectorField]].x) > 0.5f)
// HLSL: IgnoreHit();
// HLSL: = [[ahVector]].[[vectorField]].y;
// HLSL: void closestHitScalar(inout RayPayload_t{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}}, [[scalarType]] [[chScalar:[A-Za-z0-9_]+]])
// HLSL: = [[chScalar]].[[scalarField]];
// HLSL: void closestHitArray(inout RayPayload_t{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}}, [[arrayType]] [[chArray:[A-Za-z0-9_]+]])
// HLSL: = float2([[chArray]].[[arrayField]][int(0)], [[chArray]].[[arrayField]][int(1)]);
// HLSL: void closestHitStruct(inout RayPayload_t{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}}, [[userType]] {{[A-Za-z0-9_]+}})

// SM 6.7 adds access qualifiers only to the ray payload wrapper, never to an attribute wrapper.
// PAQ: struct HitAttributes_t
// PAQ-NEXT: {
// PAQ-NEXT: float {{[A-Za-z0-9_]+}}[int(2)];{{$}}
// PAQ: struct HitAttributes_t
// PAQ-NEXT: {
// PAQ-NEXT: float {{[A-Za-z0-9_]+}};{{$}}
// PAQ: struct HitAttributes_t
// PAQ-NEXT: {
// PAQ-NEXT: float2 {{[A-Za-z0-9_]+}};{{$}}
// PAQ: struct [raypayload] RayPayload_t
// PAQ-NEXT: {
// PAQ-NEXT: float4 {{[A-Za-z0-9_]+}} : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);

// DXIL-DAG: %struct.[[vector:HitAttributes_t[A-Za-z0-9_]*]] = type { <2 x float> }
// DXIL-DAG: %struct.[[scalar:HitAttributes_t[A-Za-z0-9_]*]] = type { float }
// DXIL-DAG: %struct.[[array:HitAttributes_t[A-Za-z0-9_]*]] = type { [2 x float] }
// DXIL-DAG: %struct.[[user:UserAttributes[A-Za-z0-9_]*]] = type { <2 x float> }
// DXIL: define void @{{.*}}isectVector
// DXIL: call i1 @dx.op.reportHit.struct.[[vector]](i32 158, float 1.000000e+00, i32 0, %struct.[[vector]]*
// DXIL: define void @{{.*}}isectScalar
// DXIL: call i1 @dx.op.reportHit.struct.[[scalar]](
// DXIL: define void @{{.*}}isectArray
// DXIL: call i1 @dx.op.reportHit.struct.[[array]](
// DXIL: define void @{{.*}}isectStruct
// DXIL: call i1 @dx.op.reportHit.struct.[[user]](
// DXIL: define void @{{.*}}closestHitVector{{.*}}, %struct.[[vector]]*
// DXIL: define void @{{.*}}anyHitVector{{.*}}, %struct.[[vector]]*
// DXIL: define void @{{.*}}closestHitScalar{{.*}}, %struct.[[scalar]]*
// DXIL: define void @{{.*}}closestHitArray{{.*}}, %struct.[[array]]*
// DXIL: define void @{{.*}}closestHitStruct{{.*}}, %struct.[[user]]*

// Compiled alone, the sender and each receiver still give a float2 attribute the same layout.
// SEND: struct [[sendType:HitAttributes_t[A-Za-z0-9_]*]]
// SEND-NEXT: {
// SEND-NEXT: float2 {{[A-Za-z0-9_]+}};
// SEND-NEXT: };
// SEND: [[sendType]] [[sendArg:[A-Za-z0-9_]+]] = { float2(0.25f, 0.5f) };
// SEND: ReportHit((1.0f), (0U), ([[sendArg]]))
// RECV: struct [[recvType:HitAttributes_t[A-Za-z0-9_]*]]
// RECV-NEXT: {
// RECV-NEXT: float2 [[recvField:[A-Za-z0-9_]+]];
// RECV-NEXT: };
// RECV: void {{closestHitVector|anyHitVector}}(inout RayPayload_t{{[A-Za-z0-9_]*}} {{[A-Za-z0-9_]+}}, [[recvType]] [[recvParam:[A-Za-z0-9_]+]])
// RECV: [[recvParam]].[[recvField]]
// SOLOSEND: %struct.[[sendStruct:HitAttributes_t[A-Za-z0-9_]*]] = type { <2 x float> }
// SOLOSEND: call i1 @dx.op.reportHit.struct.[[sendStruct]](i32 158, {{.*}}, %struct.[[sendStruct]]*
// SOLORECV: %struct.[[recvStruct:HitAttributes_t[A-Za-z0-9_]*]] = type { <2 x float> }
// SOLORECV: define void @{{.*}}, %struct.[[recvStruct]]* {{.*}}%attributes

// The Khronos targets keep their hit-attribute globals of the value type itself.
// SPIRV-NOT: HitAttributes_t
// SPIRV: OpVariable %{{.*}}HitAttributeKHR_v2float HitAttributeKHR
