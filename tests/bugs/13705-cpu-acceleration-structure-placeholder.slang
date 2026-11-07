// Regression test for shader-slang/slang#13705: a shader that declares a
// RaytracingAccelerationStructure but never traces a ray must still build for
// the CPU targets. The C++ emitter names the type `RaytracingAccelerationStructure`
// and the global-parameter struct keeps the field (its layout is host-visible),
// so the C++ prelude has to define the type even when nothing uses it.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=SHAREDLIB):-target shader-sharedlib -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HOSTCALLABLE):-target host-callable -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

// SHAREDLIB: result code = 0
// HOSTCALLABLE: result code = 0

// `unusedScene` is the shape from the issue: declared, never read. The CPP row pins that it
// stays in the emitted parameters, so the other rows keep exercising it.
// CPP: RaytracingAccelerationStructure unusedScene
RaytracingAccelerationStructure unusedScene;
RaytracingAccelerationStructure scene;
RaytracingAccelerationStructure sceneArray[2];

struct SceneRef
{
    RaytracingAccelerationStructure accel;
    float scale;
};

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float passThrough(RaytracingAccelerationStructure accel, float v)
{
    return v;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uniform RaytracingAccelerationStructure entryScene)
{
    // CHECK: 1.0
    outputBuffer[0] = passThrough(scene, 1.0);
    // CHECK: 2.0
    outputBuffer[1] = passThrough(entryScene, 2.0);

    SceneRef ref;
    ref.accel = sceneArray[1];
    ref.scale = 3.0;
    // CHECK: 3.0
    outputBuffer[2] = passThrough(ref.accel, ref.scale);
}
