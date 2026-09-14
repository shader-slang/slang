//TEST:SIMPLE(filecheck=METAL): -experimental-feature -allow-glsl -target metal -entry rayMain -stage raygeneration -entry fragmentMain -stage fragment

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;

    void invoke(rt::MissInput<Context> input) { input.payload.value = 1; }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

uniform rt::AccelerationStructure scene;
uniform rt::TraceProgramDescriptor<Schema> program;

// The ordinary uniform makes global-parameter collection synthesize `GlobalParams`. The global
// output varying remains a layout-only field in that wrapper's layout, while `program` requires
// the same layout to be rewritten to its schema-specific physical Metal descriptor type.
uniform float scale;
out vec4 fragmentColor;

[shader("raygeneration")]
void rayMain()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

[shader("fragment")]
void fragmentMain()
{
    fragmentColor = vec4(scale);
}

// The descriptor is physicalized even though its global layout also contains a non-nominal
// varying field. Global collection represents that varying with both a synthesized typed key and
// its original value-typed load key in the parameter group's paired layouts. Requiring both emitted
// entry points keeps those layouts and their uniform and varying producers live.
// METAL: intersectionFunctions_0;
// METAL: void rayMain(
// METAL: fragmentMain(
