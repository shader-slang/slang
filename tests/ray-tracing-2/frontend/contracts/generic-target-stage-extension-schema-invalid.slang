// Invalid concrete applications of a generic stage extension must be diagnosed through the
// schema's hit-group list and through direct entry selection, not only through explicit
// subtype queries. A class reached via the extension is reported at its own declaration. The
// first run selects no entry, so its exhaustive check pins that the schema-driven lookup
// alone reports the rule exactly once. The entry-selection run uses a fresh checking
// context, whose dedup key set does not carry over, so it may report the same rule again;
// it is checked non-exhaustively for presence only.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -entry SchemaInvalidHit -stage closesthit -no-codegen

import slang.raytracing.portable;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

interface IMarker {}

__generic<T : IMarker>
extension T : rt::IClosestHitShader
{
    typealias Context = HitContext;
    static void invoke(in rt::ClosestHitInput<Context> input) {}
}

class SchemaInvalidHit : IMarker {}
/*CHECK:
      ^^^^^^^^^^^^^^^^ invalid structural ray-tracing stage implementation
      ^^^^^^^^^^^^^^^^ structural ray-tracing stage implementation 'SchemaInvalidHit' must be declared as a struct
*/

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = SchemaInvalidHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

// The schema's hit-group list is the only reference to the invalid stage: the first test
// line selects no entry, so the diagnostic comes from the schema-driven conformance lookup
// alone. The second line selects the class directly as a stage.
struct Program : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

[numthreads(1, 1, 1)]
void main() {}
