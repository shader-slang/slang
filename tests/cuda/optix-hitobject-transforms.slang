// optix-hitobject-transforms.slang
// Test for HitObject.GetWorldToObject() and GetObjectToWorld() on CUDA/OptiX
// Regression test for https://github.com/shader-slang/slang/issues/9256

//TEST:SIMPLE(filecheck=CHECK_RAYGEN): -target cuda -entry rayGenShaderHitObjectGetWorldToObject -stage raygeneration
//TEST:SIMPLE(filecheck=CHECK_CH1): -target cuda -entry closestHitObjectToWorld3x4 -stage closesthit
//TEST:SIMPLE(filecheck=CHECK_CH2): -target cuda -entry closestHitWorldToObject3x4 -stage closesthit
//TEST:SIMPLE(filecheck=CHECK_CH3): -target cuda -entry closestHitObjectToWorld4x3 -stage closesthit
//TEST:SIMPLE(filecheck=CHECK_CH4): -target cuda -entry closestHitWorldToObject4x3 -stage closesthit

struct MatrixData
{
    float values[12];
};

struct Result
{
    MatrixData matrix;
};

uniform RaytracingAccelerationStructure sceneBVH;
uniform RWStructuredBuffer<Result> resultBuffer;

[raypayload]
struct RayPayload
{
    MatrixData matrix : read(caller) : write(caller, closesthit);;
};

MatrixData flattenFloat3x4(float3x4 value)
{
    MatrixData result;
    [unroll]
    for (uint row = 0; row < 3; ++row)
    {
        [unroll]
        for (uint column = 0; column < 4; ++column)
        {
            result.values[row * 4 + column] = value[row][column];
        }
    }
    return result;
}

MatrixData flattenFloat4x3(float4x3 value)
{
    MatrixData result;
    [unroll]
    for (uint row = 0; row < 4; ++row)
    {
        [unroll]
        for (uint column = 0; column < 3; ++column)
        {
            result.values[row * 3 + column] = value[row][column];
        }
    }
    return result;
}

static const float3 kTrianglePos = float3(0.25f, 0.25f, 1.0f);
static const float3 kObjectToWorld3x4Translation = float3(1.0f, 2.0f, 3.0f);
static const float3 kTranslatedTrianglePos = kTrianglePos + kObjectToWorld3x4Translation;

// Test 1: HitObject.GetWorldToObject() in raygen stage
// CHECK_RAYGEN: slangOptixHitObjectGetWorldToObject
[shader("raygeneration")]
void rayGenShaderHitObjectGetWorldToObject()
{
    float3 origin = {0.0, 0.0, 0.0};
    float3 dir = kTranslatedTrianglePos - origin;

    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = dir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    RayPayload payload = {};
    HitObject hit = HitObject.TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    MatrixData matrix = flattenFloat4x3(hit.GetWorldToObject());
    resultBuffer[0].matrix = matrix;
}

// Test 2: ObjectToWorld3x4() in closesthit stage
// CHECK_CH1: slangOptixGetObjectToWorldTransformMatrix
[shader("closesthit")]
void closestHitObjectToWorld3x4(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.matrix = flattenFloat3x4(ObjectToWorld3x4());
}

// Test 3: WorldToObject3x4() in closesthit stage
// CHECK_CH2: slangOptixGetWorldToObjectTransformMatrix
[shader("closesthit")]
void closestHitWorldToObject3x4(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.matrix = flattenFloat3x4(WorldToObject3x4());
}

// Test 4: ObjectToWorld4x3() in closesthit stage
// CHECK_CH3: slangOptixGetObjectToWorldTransformMatrix4x3
[shader("closesthit")]
void closestHitObjectToWorld4x3(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.matrix = flattenFloat4x3(ObjectToWorld4x3());
}

// Test 5: WorldToObject4x3() in closesthit stage
// CHECK_CH4: slangOptixGetWorldToObjectTransformMatrix4x3
[shader("closesthit")]
void closestHitWorldToObject4x3(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.matrix = flattenFloat4x3(WorldToObject4x3());
}
