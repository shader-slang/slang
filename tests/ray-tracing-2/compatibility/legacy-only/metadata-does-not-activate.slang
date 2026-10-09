//TEST:SIMPLE: -experimental-feature -entry legacyMiss -stage miss -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/compatibility/legacy-only/metadata-does-not-activate.slang -experimental-feature -o tests/ray-tracing-2/compatibility/legacy-only/metadata-values-cache.slang-module
//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/compatibility/legacy-only/metadata-values-cache.slang-module -entry legacyMiss -stage miss -no-codegen

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

T copyValue<T>(T value) { return value; }

// Declaring and constructing metadata uses ordinary Slang semantics, so it does not select
// structural dispatch or conflict with a legacy shader in the same source or serialized module.
[shader("miss")]
void legacyMiss()
{
    Schema schema = {};
    Schema copiedSchema = copyValue(schema);
}
