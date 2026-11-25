//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK): -output-using-type
//TEST:COMPARE_COMPUTE(targets=metal, filecheck-buffer=CHECK): -output-using-type -render-features argument-buffer-tier-2
// nested-parameter-block-2.slang

struct CB
{
    uint4 value;
}

struct MaterialSystem
{
    CB cb;
    RWStructuredBuffer<uint4> data;
}

struct Scene
{
    CB sceneCb;
    RWStructuredBuffer<uint4> data;
    ParameterBlock<MaterialSystem> material;
}

//TEST_INPUT: set scene = new Scene { { {1,2,3,4} }, ubuffer(data=[1 2 3 4], stride=4), new MaterialSystem {{ {1,2,3,4} }, ubuffer(data=[1 2 3 4], stride=4)} }
ParameterBlock<Scene> scene;

struct MyBuffer
{
    RWStructuredBuffer<uint4> resultBuffer;
}
//TEST_INPUT: set pb2 = new MyBuffer { out ubuffer(data=[0 0 0 0], stride=16, count=4) }
ParameterBlock<MyBuffer> pb2;

// Main entry-point. Applies the transformation encoded by `transformer`
// to all elements in `buffer`.
[shader("compute")]
[numthreads(4,1,1)]
void computeMain(uint3 sv_dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: type: uint32_t
    // CHECK-NEXT: 4
    // CHECK-NEXT: 4
    // CHECK-NEXT: 4
    // CHECK-NEXT: 4
    // CHECK-NEXT: 5
    // CHECK-NEXT: 5
    // CHECK-NEXT: 5
    // CHECK-NEXT: 5
    // CHECK-NEXT: 6
    // CHECK-NEXT: 6
    // CHECK-NEXT: 6
    // CHECK-NEXT: 6
    // CHECK-NEXT: 7
    // CHECK-NEXT: 7
    // CHECK-NEXT: 7
    // CHECK-NEXT: 7
    pb2.resultBuffer[sv_dispatchThreadID.x] = sv_dispatchThreadID.x + scene.sceneCb.value.x + scene.data[0].x + scene.material.cb.value.x + scene.material.data[0].x;
}
