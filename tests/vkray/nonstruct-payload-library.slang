// A whole library shares one D3D wrapper struct per payload value type between a dispatch and
// its receiver: TraceRay's temporary and the closesthit/miss parameters use one RayPayload_t, and
// each CallShader temporary uses the same ForceVarIntoStructTemporarily_t as the callable shader
// with that value type. CallShader copies each value into its temporary and reads it back after
// the call. The closesthit shader also re-traces its own payload and passes the wrapped data field
// to an ordinary inout helper.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile lib_6_6 -whole-program -validate-ir-detailed
//TEST:SIMPLE(filecheck=PAQ): -target hlsl -profile lib_6_7 -whole-program -validate-ir-detailed
//TEST:SIMPLE(filecheck=DXIL): -target dxil -profile lib_6_6 -whole-program
//TEST:SIMPLE(filecheck=DXIL): -target dxil -profile lib_6_7 -whole-program

RaytracingAccelerationStructure scene;
RWStructuredBuffer<float4> result;

RayDesc makeRay()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;
    return ray;
}

[noinline]
void shade(inout float4 color)
{
    color.w = 1.0;
}

[shader("raygeneration")]
void rayGen()
{
    float4 color = result[0];
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, makeRay(), color);

    float scalar = result[1].x;
    float4 vector = result[2];
    float array[4] = { result[3].x, result[3].y, result[3].z, result[3].w };
    CallShader(0, scalar);
    CallShader(1, vector);
    CallShader(2, array);

    result[4] = color;
    result[5] = float4(scalar, 0.0, 0.0, 0.0);
    result[6] = vector;
    result[7] = float4(array[0], array[1], array[2], array[3]);
}

[shader("closesthit")]
void closestHit(inout float4 payload, BuiltInTriangleIntersectionAttributes attributes)
{
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, makeRay(), payload);
    shade(payload);
}

[shader("miss")]
void miss(inout float4 payload)
{
    payload = float4(0.0);
}

[shader("callable")]
void callableScalar(inout float data)
{
    data += 1.0;
}

[shader("callable")]
void callableVector(inout float4 data)
{
    data += 1.0;
}

[shader("callable")]
void callableArray(inout float data[4])
{
    data[0] += 1.0;
}

// HLSL-DAG: struct [[rayPayload:RayPayload_t[A-Za-z0-9_]*]]{{[[:space:]]*[{][[:space:]]*}}float4 [[rayField:[A-Za-z0-9_]+]];
// HLSL-DAG: struct [[arrayData:ForceVarIntoStructTemporarily_t[A-Za-z0-9_]*]]{{[[:space:]]*[{][[:space:]]*}}float [[arrayField:[A-Za-z0-9_]+]][int(4)];
// HLSL-DAG: struct [[vectorData:ForceVarIntoStructTemporarily_t[A-Za-z0-9_]*]]{{[[:space:]]*[{][[:space:]]*}}float4 [[vectorField:[A-Za-z0-9_]+]];
// HLSL-DAG: struct [[scalarData:ForceVarIntoStructTemporarily_t[A-Za-z0-9_]*]]{{[[:space:]]*[{][[:space:]]*}}float [[scalarField:[A-Za-z0-9_]+]];
// HLSL-NOT: struct RayPayload_t
// HLSL-NOT: struct ForceVarIntoStructTemporarily_t

// HLSL: void rayGen()
// HLSL: [[rayPayload]] [[trace:[A-Za-z0-9_]+]];
// HLSL: [[trace]].[[rayField]] = [[color:[A-Za-z0-9_]+]];
// HLSL: TraceRay({{.*}}, [[trace]]);
// HLSL: [[scalarData]] [[scalarTemp:[A-Za-z0-9_]+]];
// HLSL: [[scalarTemp]].[[scalarField]] =
// HLSL: CallShader(0U, [[scalarTemp]]);
// HLSL: [[vectorData]] [[vectorTemp:[A-Za-z0-9_]+]];
// HLSL: [[vectorTemp]].[[vectorField]] =
// HLSL: CallShader(1U, [[vectorTemp]]);
// HLSL: [[arrayData]] [[arrayTemp:[A-Za-z0-9_]+]];
// HLSL: [[arrayTemp]].[[arrayField]][int(3)] =
// HLSL: CallShader(2U, [[arrayTemp]]);
// HLSL: [[trace]].[[rayField]];
// HLSL: [[scalarTemp]].[[scalarField]]
// HLSL: = [[vectorTemp]].[[vectorField]];
// HLSL: [[arrayTemp]].[[arrayField]][int(3)]

// HLSL: void [[shade:shade[A-Za-z0-9_]*]](inout float4

// HLSL: void closestHit(inout [[rayPayload]] [[hit:[A-Za-z0-9_]+]],
// HLSL: [[rayPayload]] [[retrace:[A-Za-z0-9_]+]];
// HLSL: [[retrace]].[[rayField]] = [[hit]].[[rayField]];
// HLSL: TraceRay({{.*}}, [[retrace]]);
// HLSL: [[hit]].[[rayField]] = [[retrace]].[[rayField]];
// HLSL: [[shade]]([[hit]].[[rayField]]);

// HLSL: void miss(inout [[rayPayload]] [[miss:[A-Za-z0-9_]+]])
// HLSL: [[miss]].[[rayField]] =
// HLSL: void callableScalar(inout [[scalarData]]
// HLSL: void callableVector(inout [[vectorData]]
// HLSL: void callableArray(inout [[arrayData]]

// PAQ: struct [raypayload] RayPayload_t
// PAQ-NEXT: {
// PAQ-NEXT: float4 {{[A-Za-z0-9_]+}} : read(caller, anyhit, closesthit, miss) : write(caller, anyhit, closesthit, miss);
// PAQ-NOT: [raypayload]

// DXIL-DAG: %struct.[[arrayData:ForceVarIntoStructTemporarily_t[A-Za-z0-9_]*]] = type { [4 x float] }
// DXIL-DAG: %struct.[[vectorData:ForceVarIntoStructTemporarily_t[A-Za-z0-9_]*]] = type { <4 x float> }
// DXIL-DAG: %struct.[[scalarData:ForceVarIntoStructTemporarily_t[A-Za-z0-9_]*]] = type { float }
// DXIL-DAG: %struct.[[rayPayload:RayPayload_t[A-Za-z0-9_]*]] = type { <4 x float> }
// DXIL: define void @{{.*}}rayGen
// DXIL: define void @{{.*}}closestHit{{.*}}(%struct.[[rayPayload]]*
// DXIL: define void @{{.*}}miss{{.*}}(%struct.[[rayPayload]]*
// DXIL: define void @{{.*}}callableScalar{{.*}}(%struct.[[scalarData]]*
// DXIL: define void @{{.*}}callableVector{{.*}}(%struct.[[vectorData]]*
// DXIL: define void @{{.*}}callableArray{{.*}}(%struct.[[arrayData]]*
