// Checks that a generic body's forbidden stage-input uses are diagnosed once the generic's
// type parameters become concrete, and that allowed copies keep compiling cleanly.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -target hlsl -no-codegen

import slang.raytracing.portable;

struct Payload { uint value; }

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = ::Payload;
    typealias Record = void;
}

interface IRead
{
    uint read();
}

extension rt::MissInput<MissContext> : IRead
{
    uint read() { return payload.value; }
}

interface ICarrier
{
    associatedtype Item : IRead;
}

struct Carrier : ICarrier
{
    typealias Item = rt::MissInput<MissContext>;
}

// A generic body may construct a fresh value of its own parameter while the parameter is still
// abstract. The construction must be diagnosed at the instantiation that makes it an input.
uint manufacture<T : ICarrier>()
{
    T.Item local = T.Item();
//CHECK:                 ^ fresh construction of a structural ray-tracing stage input
//CHECK:                 ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
    return local.read();
}

// Unconstrained parameters behave the same way once an instantiation names a stage input.
uint consume<T>()
{
    T t = T();
//CHECK:   ^ fresh construction of a structural ray-tracing stage input
//CHECK:   ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
    T t1;
/*CHECK:
      ^^ storage of a structural ray-tracing stage input
      ^^ ray-tracing stage input type 'rt.MissInput<MissContext>' may only be used as an 'in' parameter or an initialized local copy
*/
    T t2 = {};
//CHECK:   ^ fresh construction of a structural ray-tracing stage input
//CHECK:   ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
    return 1;
}

// Construction through more than one generic hop is diagnosed through the whole chain.
uint hopTwo<T>()
{
    T hopLocal = T();
//CHECK:          ^ fresh construction of a structural ray-tracing stage input
//CHECK:          ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
    return 1;
}

uint hopOne<T>()
{
    return hopTwo<T>();
//CHECK:   ^^^^^^ in the instantiation of 'hopTwo'
}

// A generic struct field only becomes an input after substitution; the use that supplies the
// concrete argument must reject the resulting storage, and an explicit input field stays an
// immediate error.
struct Box<T : ICarrier>
{
    T.Item value;
}

struct BadBox
{
    rt::MissInput<MissContext> value;
//CHECK:                       ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                       ^^^^^ ray-tracing stage input type 'rt.MissInput<MissContext>' may only be used as an 'in' parameter or an initialized local copy
// The rejected field's constructor access error follows the storage error.
//CHECK: declaration not accessible
//CHECK: 'MissInput.init' is not accessible from the current context.
}

void useBox(Box<Carrier> box) {}
//CHECK:                 ^^^ storage of a structural ray-tracing stage input
//CHECK:                 ^^^ ray-tracing stage input type 'Box<Carrier>' may only be used as an 'in' parameter or an initialized local copy

// Every stage-input kind reaches the same construction rule.
uint makeFresh<T>()
{
    T fresh = T();
//CHECK:       ^ fresh construction of a structural ray-tracing stage input
//CHECK:       ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
//CHECK:       ^ fresh construction of a structural ray-tracing stage input
//CHECK:       ^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' cannot be constructed here; it must come from the current invocation
//CHECK:       ^ fresh construction of a structural ray-tracing stage input
//CHECK:       ^ ray-tracing stage input type 'rt.AnyHitInput<HitContext>' cannot be constructed here; it must come from the current invocation
//CHECK:       ^ fresh construction of a structural ray-tracing stage input
//CHECK:       ^ ray-tracing stage input type 'rt.IntersectionInput<HitContext>' cannot be constructed here; it must come from the current invocation
//CHECK:       ^ fresh construction of a structural ray-tracing stage input
//CHECK:       ^ ray-tracing stage input type 'rt.CallableInput<CallableContext>' cannot be constructed here; it must come from the current invocation
    return 1;
}

// A method of a specialized generic struct is a plain call whose callee reference already
// carries the enclosing application, so the same check runs on it.
struct Factory<T>
{
    static void makeStatic()
    {
        T fresh = T();
//CHECK:           ^ fresh construction of a structural ray-tracing stage input
//CHECK:           ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
    }
}

// Passing and copying an existing input through generic helpers remains legal.
T copyInput<T>(T value) { return value; }
void passInput<T>(T value) {}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;

    static void invoke(in rt::MissInput<Context> input)
    {
        input.payload.value += manufacture<Carrier>();
//CHECK:                       ^^^^^^^^^^^ in the instantiation of 'manufacture'
        input.payload.value += consume<rt::MissInput<Context>>();
//CHECK:                       ^^^^^^^ in the instantiation of 'consume'
        input.payload.value += hopOne<rt::MissInput<Context>>();
        makeFresh<rt::MissInput<Context>>();
//CHECK:^^^^^^^^^ in the instantiation of 'makeFresh'
        Factory<rt::MissInput<Context>>::makeStatic();
//CHECK:                                           ^ in the instantiation of 'makeStatic'

        let copied = copyInput(input);
        passInput(copied);
        passInput<rt::MissInput<Context>>(input);
    }
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    static void invoke(in rt::ClosestHitInput<Context> input)
    {
        makeFresh<rt::ClosestHitInput<Context>>();
//CHECK:^^^^^^^^^ in the instantiation of 'makeFresh'
    }
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;

    static void invoke(in rt::AnyHitInput<Context> input)
    {
        makeFresh<rt::AnyHitInput<Context>>();
//CHECK:^^^^^^^^^ in the instantiation of 'makeFresh'
    }
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;

    static void invoke(in rt::IntersectionInput<Context> input)
    {
        makeFresh<rt::IntersectionInput<Context>>();
//CHECK:^^^^^^^^^ in the instantiation of 'makeFresh'
    }
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;

    static void invoke(in rt::CallableInput<Context> input)
    {
        makeFresh<rt::CallableInput<Context>>();
//CHECK:^^^^^^^^^ in the instantiation of 'makeFresh'
    }
}
