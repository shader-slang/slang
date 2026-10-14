// Stage implementations must be ordinary structs, so classes and built-in types are rejected.
// An interface may add stage requirements without itself defining an executable stage.
// Generic extension targets are also rejected for now: the check sees T before it can validate
// the concrete implementations. Supporting them is tracked in
// https://github.com/shader-slang/slang/issues/13618.
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

interface IClosestHitContract : rt::IClosestHitShader
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
