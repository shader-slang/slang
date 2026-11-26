// A search result is undefined until a match is found. The fallback makes
// every observed component defined. Demoting its aggregate phi while skipping
// the undefined incoming makes downstream SSA carry an earlier iteration's
// result around the outer loop and across TraceRay.
//
// Keep the aggregate phi and its undefined incoming in both optimization modes.
// The undefined value itself can be defined inside or outside the function.

//TEST:SIMPLE(filecheck=STRUCT): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly -validate-ir -O0 -DRESULT_KIND=0
//TEST:SIMPLE(filecheck=STRUCT): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly -validate-ir -O0 -DRESULT_KIND=0 -minimum-slang-optimization
//TEST:SIMPLE(filecheck=ARRAY): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly -validate-ir -O0 -DRESULT_KIND=1
//TEST:SIMPLE(filecheck=ARRAY): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly -validate-ir -O0 -DRESULT_KIND=1 -minimum-slang-optimization
//TEST:SIMPLE(filecheck=MATRIX): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly -validate-ir -O0 -DRESULT_KIND=2
//TEST:SIMPLE(filecheck=MATRIX): -target spirv-asm -stage raygeneration -entry main -emit-spirv-directly -validate-ir -O0 -DRESULT_KIND=2 -minimum-slang-optimization

#if RESULT_KIND == 0
struct Result
{
    float first;
    float second;
};
Result makeResult(float value)
{
    return {value, value + 1};
}
float sumResult(Result value)
{
    return value.first + value.second;
}
#elif RESULT_KIND == 1
typealias Result = float[2];
Result makeResult(float value)
{
    return {value, value + 1};
}
float sumResult(Result value)
{
    return value[0] + value[1];
}
#else
typealias Result = float2x2;
Result makeResult(float value)
{
    return float2x2(value, value + 1, value + 2, value + 3);
}
float sumResult(Result value)
{
    return value[0][0] + value[0][1] + value[1][0] + value[1][1];
}
#endif

RaytracingAccelerationStructure scene;
RWStructuredBuffer<float> buf;

[shader("raygeneration")]
void main()
{
    RayDesc ray = {0, 0, 1, 1e30};
    float payload = 0;
    for (uint bounce = 0; bounce < 4; ++bounce)
    {
        TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload);
        Result result;
        bool found = false;
        for (uint index = 0; index < 4; ++index)
        {
            float value = buf[index];
            if (payload < value)
            {
                result = makeResult(value);
                found = true;
                break;
            }
        }
        if (!found)
            result = makeResult(1);
        ray.TMax = sumResult(result);
    }
    buf[0] = ray.TMax;
}

// STRUCT-DAG: %[[TYPE:[A-Za-z0-9_]+]] = OpTypeStruct %float %float
// STRUCT-DAG: %[[UNDEF:[A-Za-z0-9_]+]] = OpUndef %[[TYPE]]
// STRUCT-DAG: OpTraceRayKHR
// STRUCT: OpPhi %[[TYPE]] {{.*}}%[[UNDEF]]

// ARRAY-DAG: %[[TYPE:[A-Za-z0-9_]+]] = OpTypeArray %float
// ARRAY-DAG: %[[UNDEF:[A-Za-z0-9_]+]] = OpUndef %[[TYPE]]
// ARRAY-DAG: OpTraceRayKHR
// ARRAY: OpPhi %[[TYPE]] {{.*}}%[[UNDEF]]

// MATRIX-DAG: %[[TYPE:[A-Za-z0-9_]+]] = OpTypeMatrix %v2float 2
// MATRIX-DAG: %[[UNDEF:[A-Za-z0-9_]+]] = OpUndef %[[TYPE]]
// MATRIX-DAG: OpTraceRayKHR
// MATRIX: OpPhi %[[TYPE]] {{.*}}%[[UNDEF]]
