// D3D accepts only a struct as a receiving shader's ray or callable payload. A non-struct payload
// parameter becomes a one-field wrapper struct, and the body reads and writes its data field in
// place. The anyhit shader checks that a write before AcceptHitAndEndSearch goes to the payload
// itself, because that call does not return to run a copy-out.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile lib_6_6 -whole-program -validate-ir-detailed
//TEST:SIMPLE(filecheck=PAQ): -target hlsl -profile lib_6_7 -whole-program -validate-ir-detailed
//TEST:SIMPLE(filecheck=DXIL): -target dxil -profile lib_6_6 -whole-program
//TEST:SIMPLE(filecheck=DXIL): -target dxil -profile lib_6_7 -whole-program

RWStructuredBuffer<float> result;

[shader("anyhit")]
void anyHitVector(inout float4 payload, BuiltInTriangleIntersectionAttributes attributes)
{
    payload.x = attributes.barycentrics.x;
    if (payload.y > 0.5)
        AcceptHitAndEndSearch();
    payload.z = 1.0;
}

[shader("closesthit")]
void closestHitVector(inout float4 payload, BuiltInTriangleIntersectionAttributes attributes)
{
    payload = float4(attributes.barycentrics, RayTCurrent(), 1.0);
}

[shader("closesthit")]
void closestHitScalar(inout float payload, BuiltInTriangleIntersectionAttributes attributes)
{
    payload += RayTCurrent();
}

[shader("miss")]
void missVector(inout float4 payload)
{
    payload = float4(0.0, 0.0, 0.0, 1.0);
}

[shader("miss")]
void missArray(inout float payload[4])
{
    payload[2] = result[0];
}

[shader("callable")]
void callableVector(inout float4 data)
{
    data *= 2.0;
}

// HLSL: struct [[vector:RayPayload_t[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: float4 [[vectorField:[A-Za-z0-9_]+]];
// HLSL: void anyHitVector(inout [[vector]] [[anyHit:[A-Za-z0-9_]+]],
// HLSL: [[anyHit]].[[vectorField]][int(0)] = {{.*}}barycentrics.x;
// HLSL: AcceptHitAndEndSearch();
// HLSL: [[anyHit]].[[vectorField]][int(2)] = 1.0f;
// HLSL: void closestHitVector(inout [[vector]] [[closestHit:[A-Za-z0-9_]+]],
// HLSL: [[closestHit]].[[vectorField]] = float4(
// HLSL: struct [[scalar:RayPayload_t[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: float [[scalarField:[A-Za-z0-9_]+]];
// HLSL: void closestHitScalar(inout [[scalar]] [[closestHitScalar:[A-Za-z0-9_]+]],
// HLSL: [[closestHitScalar]].[[scalarField]] = [[closestHitScalar]].[[scalarField]] +
// HLSL: void missVector(inout [[vector]] [[miss:[A-Za-z0-9_]+]])
// HLSL: [[miss]].[[vectorField]] = float4(
// HLSL: struct [[array:RayPayload_t[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: float [[arrayField:[A-Za-z0-9_]+]][int(4)];
// HLSL: void missArray(inout [[array]] [[missArray:[A-Za-z0-9_]+]])
// HLSL: [[missArray]].[[arrayField]][int(2)] =
// HLSL: struct [[callable:ForceVarIntoStructTemporarily_t[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: float4 [[callableField:[A-Za-z0-9_]+]];
// HLSL: void callableVector(inout [[callable]] [[callableData:[A-Za-z0-9_]+]])
// HLSL: [[callableData]].[[callableField]] = [[callableData]].[[callableField]] * 2.0f;

// SM 6.7 requires access qualifiers on each [raypayload] field. Callable data is not a ray
// payload, so its wrapper has neither the attribute nor qualifiers.
// PAQ: struct [raypayload] RayPayload_t
// PAQ-NEXT: {
// PAQ-NEXT: float4 {{[A-Za-z0-9_]+}} : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// PAQ: struct [raypayload] RayPayload_t
// PAQ-NEXT: {
// PAQ-NEXT: float {{[A-Za-z0-9_]+}} : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// PAQ: struct [raypayload] RayPayload_t
// PAQ-NEXT: {
// PAQ-NEXT: float {{[A-Za-z0-9_]+}}[int(4)] : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// PAQ: struct ForceVarIntoStructTemporarily_t
// PAQ-NEXT: {
// PAQ-NEXT: float4 {{[A-Za-z0-9_]+}};

// DXIL: %struct.RayPayload_t{{[A-Za-z0-9_]*}} = type { <4 x float> }
// DXIL: %struct.RayPayload_t{{[A-Za-z0-9_]*}} = type { float }
// DXIL: %struct.RayPayload_t{{[A-Za-z0-9_]*}} = type { [4 x float] }
// DXIL: %struct.ForceVarIntoStructTemporarily_t{{[A-Za-z0-9_]*}} = type { <4 x float> }
// DXIL: define void @{{.*}}anyHitVector{{.*}}(%struct.RayPayload_t
// DXIL: define void @{{.*}}missArray{{.*}}(%struct.RayPayload_t
// DXIL: define void @{{.*}}callableVector{{.*}}(%struct.ForceVarIntoStructTemporarily_t
