// Empty logical payloads must not acquire physical fields as they cross ordinary helpers.
// Each helper retains its scalar argument, branches, and atomic writes. Only the actual
// dispatch needs the canonical one-uint interface object. A receiving entry point must also
// be able to forward its empty logical input through the same helpers without copying a dummy.
// __ref plus noRefInline opts out of the separate reference-parameter inlining pass.

//TEST:SIMPLE(filecheck=GLSL,TRACE): -entry main -stage raygeneration -target glsl -O0 -DUSE_CALLABLE=0 -DRECEIVER=0 -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=GLSL,TRACE): -entry main -stage raygeneration -target glsl -O3 -DUSE_CALLABLE=0 -DRECEIVER=0 -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=GLSL,CALL): -entry main -stage raygeneration -target glsl -O0 -DUSE_CALLABLE=1 -DRECEIVER=0 -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=GLSL,CALL): -entry main -stage raygeneration -target glsl -O3 -DUSE_CALLABLE=1 -DRECEIVER=0 -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=GLSL,TRACE,RECEIVER): -entry main -stage miss -target glsl -O0 -DUSE_CALLABLE=0 -DRECEIVER=1 -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=GLSL,CALL,RECEIVER): -entry main -stage callable -target glsl -O0 -DUSE_CALLABLE=1 -DRECEIVER=1 -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=NO_COPY): -entry main -stage miss -target glsl -O0 -DUSE_CALLABLE=0 -DRECEIVER=1 -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=NO_COPY): -entry main -stage callable -target glsl -O0 -DUSE_CALLABLE=1 -DRECEIVER=1 -line-directive-mode none -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage miss -target spirv -emit-spirv-via-glsl -DUSE_CALLABLE=0 -DRECEIVER=1 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage callable -target spirv -emit-spirv-via-glsl -DUSE_CALLABLE=1 -DRECEIVER=1 -validate-ir

struct Empty
{
}

struct Payload
{
    Empty inner;
    Empty values[2];
}

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> events;

[noinline]
[noRefInline]
void dispatchLeaf(__ref Payload data, __ref Empty emptyArray[2], uint tag)
{
    InterlockedAdd(events[0], tag);
#if USE_CALLABLE
    CallShader(0, data);
#else
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1.0;
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, data);
#endif
    InterlockedAdd(events[1], tag);
}

[noinline]
[noRefInline]
void forwardMiddle(__ref Payload data, __ref Empty emptyArray[2], uint tag)
{
    InterlockedAdd(events[2], tag);
    if ((tag & 1) != 0)
    {
        InterlockedAdd(events[6], tag);
        dispatchLeaf(data, emptyArray, tag + 1);
    }
    else
        dispatchLeaf(data, emptyArray, tag + 2);
    InterlockedAdd(events[3], tag);
}

[noinline]
[noRefInline]
void forwardOuter(__ref Payload data, __ref Empty emptyArray[2], uint tag)
{
    InterlockedAdd(events[4], tag);
    forwardMiddle(data, emptyArray, tag);
    InterlockedAdd(events[5], tag);
}

#if RECEIVER
#if USE_CALLABLE
[shader("callable")]
#else
[shader("miss")]
#endif
void main(inout Payload data)
#else
[shader("raygeneration")]
void main()
#endif
{
#if !RECEIVER
    Payload data;
#endif
    Empty emptyArray[2];
    uint tag = events[7];
    InterlockedAdd(events[8], tag);
    forwardOuter(data, emptyArray, tag);
    InterlockedAdd(events[9], tag);
}

// The physical type contains exactly one uint. Neither original logical type is emitted.
// GLSL-NOT: struct Payload
// GLSL-NOT: struct Empty_
// GLSL: struct [[physicalType:Empty(Ray|Callable)Payload[A-Za-z0-9_]*]]
// GLSL-NEXT: {
// GLSL-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// GLSL-NEXT: };
// GLSL-NOT: struct Payload
// GLSL-NOT: struct Empty_
// GLSL-DAG: {{rayPayloadEXT|callableDataEXT}}
// RECEIVER-DAG: {{rayPayloadInEXT|callableDataInEXT}}
// GLSL-DAG: [[physicalType]] {{[A-Za-z0-9_]+}};
// GLSL-NOT: struct Payload
// GLSL-NOT: struct Empty_

// CallShader's standard-library wrapper may remain, but it also loses its empty parameter.
// Check its real native dispatch and then the leaf's call, without requiring wrapper inlining.
// CALL: void [[native:CallShader[A-Za-z0-9_]*]](uint [[shaderIndex:[A-Za-z0-9_]+]])
// CALL: executeCallableEXT([[shaderIndex]], {{[0-9]+}});

// Exact scalar-only signatures and calls prevent the physical object leaking into helpers.
// GLSL: void [[leaf:dispatchLeaf[A-Za-z0-9_]*]](uint {{[A-Za-z0-9_]+}})
// GLSL: atomicAdd({{.*}}[{{uint\(0\)|0}}],
// TRACE: traceRayEXT(
// CALL: [[native]](0U);
// GLSL: atomicAdd({{.*}}[{{uint\(1\)|1}}],
// GLSL: void [[middle:forwardMiddle[A-Za-z0-9_]*]](uint {{[A-Za-z0-9_]+}})
// GLSL: atomicAdd({{.*}}[{{uint\(2\)|2}}],
// GLSL: if
// GLSL: atomicAdd({{.*}}[{{uint\(6\)|6}}],
// GLSL: [[leaf]]({{[^,;]+}});
// GLSL: [[leaf]]({{[^,;]+}});
// GLSL: atomicAdd({{.*}}[{{uint\(3\)|3}}],
// GLSL: void [[outer:forwardOuter[A-Za-z0-9_]*]](uint {{[A-Za-z0-9_]+}})
// GLSL: atomicAdd({{.*}}[{{uint\(4\)|4}}],
// GLSL: [[middle]]({{[^,;]+}});
// GLSL: atomicAdd({{.*}}[{{uint\(5\)|5}}],
// GLSL: void main()
// GLSL: atomicAdd({{.*}}[{{uint\(8\)|8}}],
// GLSL: [[outer]]({{[^,;]+}});
// GLSL: atomicAdd({{.*}}[{{uint\(9\)|9}}],

// A separate check spans the whole emitted body. GLSL dispatches address payloads by location,
// so the outgoing physical global must not appear in logical copy-in or copy-out statements.
// NO_COPY-NOT: struct Payload
// NO_COPY-NOT: struct Empty_
// NO_COPY: {{rayPayloadEXT|callableDataEXT}}
// NO_COPY-NEXT: {{[A-Za-z0-9_]+}} [[physical:[A-Za-z0-9_]+]];
// NO_COPY-NOT: {{\b}}[[physical]]{{\b}}
// NO_COPY-NOT: struct Payload
// NO_COPY-NOT: struct Empty_

// Compile the receiving forms through GLSL as well, to validate the complete shader interface.
// SPIRV: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// SPIRV: %[[physicalType:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// SPIRV: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer {{RayPayloadKHR|CallableDataKHR}} %[[physicalType]]
// SPIRV: %[[physical:[A-Za-z0-9_]+]] = OpVariable %[[pointer]] {{RayPayloadKHR|CallableDataKHR}}
// SPIRV: Op{{TraceRayKHR|ExecuteCallableKHR}} {{.*}} %[[physical]]{{$}}
