//TEST:INTERPRET(hgvm): -api default

void fail(String message)
{
    hgbc_asm {
        OpFail $message;
    };
}

void check(bool condition, String message)
{
    if (!condition)
        fail(message);
}

struct EmptyTag
{}

struct Camera
{
    EmptyTag beforeView;
    row_major float4x4 view;
    EmptyTag beforeProjection;
    row_major float4x4 projection;
}

struct CameraData
{
    Camera camera;
    EmptyTag beforeScreenCenter;
    float2 screenCenter;
}

struct BindingResources
{
    EmptyTag beforeObservations;
    RWStructuredBuffer<float> observations;
}

struct BindingData
{
    float value;
    EmptyTag beforeResources;
    BindingResources resources;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void inspectAndModifyCamera(
    RWStructuredBuffer<CameraData> cameraData,
    RWStructuredBuffer<float> observations)
{
    CameraData value = cameraData[0];
    observations[0] = value.camera.view[0][1];
    observations[1] = value.camera.view[2][3];
    observations[2] = value.camera.projection[1][2];
    observations[3] = value.screenCenter.y;
    value.camera.projection[3][0] = 91.0f;
    cameraData[0] = value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void inspectBindingData(BindingData data)
{
    data.resources.observations[4] = data.value;
}

[host]
void main()
{
    CameraData value;
    value.camera.view = float4x4(
        float4(1.0f, 2.0f, 3.0f, 4.0f),
        float4(5.0f, 6.0f, 7.0f, 8.0f),
        float4(9.0f, 10.0f, 11.0f, 12.0f),
        float4(13.0f, 14.0f, 15.0f, 16.0f));
    value.camera.projection = float4x4(
        float4(21.0f, 22.0f, 23.0f, 24.0f),
        float4(25.0f, 26.0f, 27.0f, 28.0f),
        float4(29.0f, 30.0f, 31.0f, 32.0f),
        float4(33.0f, 34.0f, 35.0f, 36.0f));
    value.screenCenter = float2(0.25f, 0.75f);

    RuntimeArray<CameraData> source = RuntimeArray<CameraData>::alloc(1);
    source[0] = value;
    RWStructuredBuffer<CameraData> destination = RWStructuredBuffer<CameraData>::alloc(1);
    copy(destination, 0, source, 0, 1);

    RWStructuredBuffer<float> observations = RWStructuredBuffer<float>::alloc(5);
    dispatch(inspectAndModifyCamera, uint3(1, 1, 1))(destination, observations);

    BindingData bindingData;
    bindingData.value = 53.0f;
    bindingData.resources.observations = observations;
    dispatch(inspectBindingData, uint3(1, 1, 1))(bindingData);

    check(readBack(observations, 0) == 2.0f, "HGVM changed matrix row/column ordering");
    check(readBack(observations, 1) == 12.0f, "HGVM uploaded the wrong nested matrix value");
    check(readBack(observations, 2) == 27.0f, "HGVM uploaded the wrong projection value");
    check(readBack(observations, 3) == 0.75f, "HGVM uploaded the wrong trailing field");
    check(
        readBack(observations, 4) == 53.0f,
        "HGVM did not match compact host fields to shader reflection by name");

    CameraData modified = readBack(destination, 0);
    check(
        modified.camera.projection[3][0] == 91.0f,
        "HGVM readback did not convert the nested matrix layout");
}
