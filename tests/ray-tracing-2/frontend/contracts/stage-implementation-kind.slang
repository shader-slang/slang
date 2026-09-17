// Structural stages use compiler-created value receivers. Classes, built-in types, and generic
// parameters do not provide the required concrete value representation, so reject them when their
// conformances are checked. An interface inheriting the stage contract remains legal because open
// sections use that pattern as a tag rather than as an executable stage.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

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

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

interface IClosestHitTag : rt::IClosestHitShader
{
}

class InvalidClosestHit : rt::IClosestHitShader
/*CHECK:
      ^^^^^^^^^^^^^^^^^ invalid structural ray-tracing stage implementation
      ^^^^^^^^^^^^^^^^^ structural ray-tracing stage implementation 'InvalidClosestHit' must be
   declared as a struct
*/
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<Context> input) {}
}

extension float : rt::IClosestHitShader
//CHECK:              ^^^^^^^^^^^^^^^^^ invalid structural ray-tracing stage implementation
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<Context> input) {}
}

interface IGenericStageTarget {}

__generic<T : IGenericStageTarget> extension T : rt::IClosestHitShader
//CHECK:                                             ^^^^^^^^^^^^^^^^^ invalid structural ray-tracing stage implementation
//CHECK:                                             ^^^^^^^^^^^^^^^^^ structural ray-tracing stage implementation 'T' must be declared as a struct
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<Context> input) {}
}
