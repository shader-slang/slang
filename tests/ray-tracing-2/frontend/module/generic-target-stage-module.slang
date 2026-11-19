// The generic stage extension lives in a separate module from the concrete types it applies
// to. Both a source import and a serialized module import must carry the extension and its
// deferred user-defined-struct rule, and concrete generic specializations are checked too.
//TEST:SIMPLE: -experimental-feature -entry Hit -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Hit.invoke -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry HitBox<Hit> -stage closesthit -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/generic_target_stage_library.slang -experimental-feature -o tests/ray-tracing-2/frontend/module/generic-target-stage-cache.slang-module
//TEST:SIMPLE: -experimental-feature -r tests/ray-tracing-2/frontend/module/generic-target-stage-cache.slang-module -entry Hit -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -r tests/ray-tracing-2/frontend/module/generic-target-stage-cache.slang-module -entry HitBox<Hit> -stage closesthit -no-codegen

import slang.raytracing.portable;
import generic_target_stage_library;

// The stage type and the extension that implements its stage interface live in different
// modules. Ordinary data fields stay ordinary struct data.
struct Hit : stage::IMarker
{
    uint state;
}

struct HitBox<T : stage::IMarker> : stage::IMarker
{
    T inner;
}

// A hit group reached only through a schema exercises the schema conformance lookup across the
// module boundary.
struct Program : rt::ITraceProgramSchema
{
    typealias TraceContext = stage::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = stage::HitContext;
    typealias ClosestHit = Hit;
    typealias AnyHit = rt::NoAnyHit<stage::HitContext>;
    typealias Intersection = rt::NoIntersection<stage::HitContext>;
}

[numthreads(1, 1, 1)]
void main() {}
