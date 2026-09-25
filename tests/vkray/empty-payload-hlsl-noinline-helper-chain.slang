// D3D payload reconstruction belongs at the actual dispatch, not at an ordinary helper call.
// Nested empty structs and arrays disappear from all three noinline helper signatures, while
// the branches and atomic writes around the dispatch remain in their original functions.
// __ref plus noRefInline also opts out of the separate reference-parameter inlining pass.

//TEST:SIMPLE(filecheck=HLSL): -entry main -stage raygeneration -target hlsl -profile sm_6_6 -O0 -DUSE_CALLABLE=0 -validate-ir
//TEST:SIMPLE(filecheck=HLSL): -entry main -stage raygeneration -target hlsl -profile sm_6_6 -O3 -DUSE_CALLABLE=0 -validate-ir
//TEST:SIMPLE(filecheck=HLSL): -entry main -stage raygeneration -target hlsl -profile sm_6_6 -O0 -DUSE_CALLABLE=1 -validate-ir
//TEST:SIMPLE(filecheck=HLSL): -entry main -stage raygeneration -target hlsl -profile sm_6_6 -O3 -DUSE_CALLABLE=1 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -entry main -stage raygeneration -target dxil -profile sm_6_6 -DUSE_CALLABLE=0 -validate-ir
//TEST:SIMPLE(filecheck=DXIL): -entry main -stage raygeneration -target dxil -profile sm_6_6 -DUSE_CALLABLE=1 -validate-ir

struct Empty
{
}

struct Payload
{
    Empty values[2];
}

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> events;

[noinline]
[noRefInline]
void dispatchLeaf(__ref Payload rayData, __ref Empty callableData[2], uint tag)
{
    InterlockedAdd(events[0], tag);
#if USE_CALLABLE
    CallShader(0, callableData);
#else
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, rayData);
#endif
    InterlockedAdd(events[1], tag);
}

[noinline]
[noRefInline]
void forwardMiddle(__ref Payload rayData, __ref Empty callableData[2], uint tag)
{
    InterlockedAdd(events[2], tag);
    if ((tag & 1) != 0)
    {
        InterlockedAdd(events[6], tag);
        dispatchLeaf(rayData, callableData, tag + 1);
    }
    else
        dispatchLeaf(rayData, callableData, tag + 2);
    InterlockedAdd(events[3], tag);
}

[noinline]
[noRefInline]
void forwardOuter(__ref Payload rayData, __ref Empty callableData[2], uint tag)
{
    InterlockedAdd(events[4], tag);
    forwardMiddle(rayData, callableData, tag);
    InterlockedAdd(events[5], tag);
}

[shader("raygeneration")]
void main()
{
    Payload rayData;
    Empty callableData[2];
    forwardOuter(rayData, callableData, DispatchRaysIndex().x);
}

// HLSL: struct {{(\[raypayload\] )?}}[[payloadType:Empty(Ray|Callable)Payload[A-Za-z0-9_]*]]
// HLSL-NEXT: {
// HLSL-NEXT: uint _slang_dummy{{.*}};
// HLSL-NEXT: };
// HLSL: void [[leaf:dispatchLeaf[A-Za-z0-9_]*]](uint {{[A-Za-z0-9_]+}})
// HLSL: InterlockedAdd
// HLSL: [[payloadType]] [[payload:[A-Za-z0-9_]+]];
// HLSL: {{TraceRay|CallShader}}({{.*}}[[payload]]);
// HLSL: InterlockedAdd
// HLSL: void [[middle:forwardMiddle[A-Za-z0-9_]*]](uint {{[A-Za-z0-9_]+}})
// HLSL: InterlockedAdd
// HLSL: if
// HLSL: [[leaf]]({{[^,;]+}});
// HLSL: [[leaf]]({{[^,;]+}});
// HLSL: InterlockedAdd
// HLSL: void [[outer:forwardOuter[A-Za-z0-9_]*]](uint {{[A-Za-z0-9_]+}})
// HLSL: InterlockedAdd
// HLSL: [[middle]]({{[^,;]+}});
// HLSL: InterlockedAdd
// HLSL: void main()
// HLSL: [[outer]]({{[^,;]+}});

// DXIL: %struct.Empty{{Ray|Callable}}Payload{{[A-Za-z0-9_]*}} = type { i32 }
// DXIL: define internal fastcc void {{.*}}dispatchLeaf{{.*}}(i32 %{{[A-Za-z0-9_]+}})
// DXIL: call void @dx.op.{{traceRay|callShader}}.
// DXIL: define internal fastcc void {{.*}}forwardMiddle{{.*}}(i32 %{{[A-Za-z0-9_]+}})
// DXIL: call fastcc void {{.*}}dispatchLeaf
// DXIL: define internal fastcc void {{.*}}forwardOuter{{.*}}(i32 %{{[A-Za-z0-9_]+}})
// DXIL: call fastcc void {{.*}}forwardMiddle
// DXIL: define void {{.*}}main
// DXIL: call fastcc void {{.*}}forwardOuter
