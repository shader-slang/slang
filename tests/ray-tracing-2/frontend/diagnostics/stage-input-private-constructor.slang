//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -no-codegen

import slang.raytracing.portable;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Context : rt::IHitContext, rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Record = void;
    typealias Primitive = rt::TrianglePrimitive;
    typealias CallableData = uint;
}

// Each stage input must reject both explicit constructor calls and empty initializer lists
// through ordinary constructor access checking.
void cannotConstructStageInputs()
{
    rt::ClosestHitInput<Context>();
//CHECK:                        ^ declaration not accessible
//CHECK:                        ^ 'ClosestHitInput.init' is not accessible from the current context.
    rt::AnyHitInput<Context>();
//CHECK:                    ^ declaration not accessible
//CHECK:                    ^ 'AnyHitInput.init' is not accessible from the current context.
    rt::IntersectionInput<Context>();
//CHECK:                          ^ declaration not accessible
//CHECK:                          ^ 'IntersectionInput.init' is not accessible from the current context.
    rt::MissInput<Context>();
//CHECK:                  ^ declaration not accessible
//CHECK:                  ^ 'MissInput.init' is not accessible from the current context.
    rt::CallableInput<Context>();
//CHECK:                      ^ declaration not accessible
//CHECK:                      ^ 'CallableInput.init' is not accessible from the current context.

    rt::ClosestHitInput<Context> closest = {};
//CHECK:                                   ^ declaration not accessible
//CHECK:                                   ^ 'ClosestHitInput.init' is not accessible from the current context.
    rt::AnyHitInput<Context> any = {};
//CHECK:                           ^ declaration not accessible
//CHECK:                           ^ 'AnyHitInput.init' is not accessible from the current context.
    rt::IntersectionInput<Context> intersection = {};
//CHECK:                                          ^ declaration not accessible
//CHECK:                                          ^ 'IntersectionInput.init' is not accessible from the current context.
    rt::MissInput<Context> miss = {};
//CHECK:                          ^ declaration not accessible
//CHECK:                          ^ 'MissInput.init' is not accessible from the current context.
    rt::CallableInput<Context> callable = {};
//CHECK:                                  ^ declaration not accessible
//CHECK:                                  ^ 'CallableInput.init' is not accessible from the current context.
}
