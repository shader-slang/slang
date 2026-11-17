// Stage implementations must be ordinary structs, so classes and built-in types are rejected.
// An interface may add stage requirements without itself defining an executable stage.
// A generic extension that names its own parameter as the target is accepted; the
// user-defined-struct rule is then applied to each concrete type the extension applies to.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

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

interface IClosestHitContract : rt::IClosestHitShader
{
}

class InvalidClosestHit : rt::IClosestHitShader
/*CHECK:
      ^^^^^^^^^^^^^^^^^ invalid structural ray-tracing stage implementation
      ^^^^^^^^^^^^^^^^^ structural ray-tracing stage implementation 'InvalidClosestHit' must be declared as a struct
*/
{
    typealias Context = HitContext;
    static void invoke(rt::ClosestHitInput<Context> input) {}
}

extension float : rt::IClosestHitShader
//CHECK:              ^^^^^^^^^^^^^^^^^ invalid structural ray-tracing stage implementation
//CHECK:              ^^^^^^^^^^^^^^^^^ structural ray-tracing stage implementation 'float' must be declared as a struct
{
    typealias Context = HitContext;
    static void invoke(rt::ClosestHitInput<Context> input) {}
}

interface IGenericStageTarget {}

// The generic extension itself is accepted while its target is still the parameter T. The
// annotations below it are the diagnostic for float: main's useStage<float>() applies this
// extension to float, and the location is this clause because a builtin type has no source
// declaration of its own to point at.
__generic<T : IGenericStageTarget>
extension T : rt::IClosestHitShader
//CHECK:          ^^^^^^^^^^^^^^^^^ invalid structural ray-tracing stage implementation
//CHECK:          ^^^^^^^^^^^^^^^^^ structural ray-tracing stage implementation 'float' must be declared as a struct
{
    typealias Context = HitContext;
    static void invoke(in rt::ClosestHitInput<Context> input) {}
}

// Concrete applications of the generic extension are checked when the extension applies to the
// type, so a builtin and a class reached through it still receive the user-defined-struct rule.
extension float : IGenericStageTarget {}
class InvalidGenericTarget : IGenericStageTarget {}
/*CHECK:
      ^^^^^^^^^^^^^^^^^^^^ invalid structural ray-tracing stage implementation
      ^^^^^^^^^^^^^^^^^^^^ structural ray-tracing stage implementation 'InvalidGenericTarget' must be declared as a struct
*/

// Selecting a type as a stage computes its inheritance information, which applies the generic
// extension and runs the deferred rule.
void useStage<T : rt::IClosestHitShader>() {}

[numthreads(1, 1, 1)]
void main()
{
    useStage<float>();
    useStage<InvalidGenericTarget>();
}
