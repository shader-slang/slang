//TEST:SIMPLE(filecheck=EMPTY): -experimental-feature -target hlsl -whole-program -DLIST_KIND=0
//TEST:SIMPLE(filecheck=EMPTY): -experimental-feature -target hlsl -whole-program -DLIST_KIND=1
//TEST:SIMPLE(filecheck=NONEMPTY): -experimental-feature -target hlsl -whole-program -DLIST_KIND=2

// Both generic list constraints and a generic schema must expose compile-time counts. Only the
// counts reach code generation, so this test does not require structural stage lowering. Each
// export reads one stage's counts; its generic list specialization leaves other sections empty
// so references to concrete shader types do not combine incompatible stage requirements.
//EMPTY-COUNT-6: return int(0);
//NONEMPTY-DAG: return int(1);
//NONEMPTY-DAG: return int(1);
//NONEMPTY-DAG: return int(2);
//NONEMPTY-DAG: return int(2);
//NONEMPTY-DAG: return int(3);
//NONEMPTY-DAG: return int(3);
import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Record = void;
}

struct Miss<let index : int> : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<Context> input) {}
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = uint;
    typealias Record = void;
}

struct Callable<let index : int> : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<Context> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
#if LIST_KIND == 0
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
#elif LIST_KIND == 1
    typealias HitGroups = rt::HitGroupList<>;
    typealias MissShaders = rt::MissShaderList<>;
    typealias CallableShaders = rt::CallableShaderList<>;
#else
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<Miss<0>, Miss<1>>;
    typealias CallableShaders = rt::CallableShaderList<Callable<0>, Callable<1>, Callable<2>>;
#endif
}

// These generic schemas consume the list and schema types only as compile-time metadata.
struct ListCounts<Hits : rt::IHitGroupList,
                  Misses : rt::IMissShaderList,
                  Callables : rt::ICallableShaderList> : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = Hits;
    typealias MissShaders = Misses;
    typealias CallableShaders = Callables;

    static const int hitCount = Hits.count;
    static const int missCount = Misses.count;
    static const int callableCount = Callables.count;
}

struct SchemaCounts<S : rt::ITraceProgramSchema> : rt::ITraceProgramSchema
{
    typealias TraceContext = S.TraceContext;
    typealias HitGroups = S.HitGroups;
    typealias MissShaders = S.MissShaders;
    typealias CallableShaders = S.CallableShaders;

    static const int hitCount = S.HitGroups.count;
    static const int missCount = S.MissShaders.count;
    static const int callableCount = S.CallableShaders.count;
}

[require(structural_raytracing_closesthit)]
export int hitListCount()
{
    typealias Counts =
        ListCounts<Schema.HitGroups, rt::EmptyMissShaderList, rt::EmptyCallableShaderList>;
    static_assert(Counts.hitCount == (LIST_KIND == 2 ? 1 : 0), "Hit list count");
    // Reserve a sentinel element so an empty list does not require a zero-length array.
    int entries[Counts.hitCount + 1] = {};
    return entries.getCount() - 1;
}

[require(structural_raytracing_closesthit)]
export int hitSchemaCount()
{
    typealias Counts = SchemaCounts<Schema>;
    return Counts.hitCount;
}

[require(structural_raytracing_miss)]
export int missListCount()
{
    typealias Counts =
        ListCounts<rt::EmptyHitGroupList, Schema.MissShaders, rt::EmptyCallableShaderList>;
    static_assert(Counts.missCount == (LIST_KIND == 2 ? 2 : 0), "Miss list count");
    // Reserve a sentinel element so an empty list does not require a zero-length array.
    int entries[Counts.missCount + 1] = {};
    return entries.getCount() - 1;
}

[require(structural_raytracing_miss)]
export int missSchemaCount()
{
    typealias Counts = SchemaCounts<Schema>;
    return Counts.missCount;
}

[require(structural_raytracing_callable)]
export int callableListCount()
{
    typealias Counts =
        ListCounts<rt::EmptyHitGroupList, rt::EmptyMissShaderList, Schema.CallableShaders>;
    static_assert(Counts.callableCount == (LIST_KIND == 2 ? 3 : 0), "Callable list count");
    // Reserve a sentinel element so an empty list does not require a zero-length array.
    int entries[Counts.callableCount + 1] = {};
    return entries.getCount() - 1;
}

[require(structural_raytracing_callable)]
export int callableSchemaCount()
{
    typealias Counts = SchemaCounts<Schema>;
    return Counts.callableCount;
}
