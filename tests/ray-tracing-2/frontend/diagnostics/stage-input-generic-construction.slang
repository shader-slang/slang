// Checks that a generic body's forbidden stage-input uses are diagnosed once the generic's
// type parameters become concrete, and that allowed copies keep compiling cleanly. The
// SPECIALIZE run checks the same rule when the concrete argument comes from -specialize on a
// native generic entry point instead of a call.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=SPECIALIZE,non-exhaustive): -experimental-feature -ignore-capabilities -target hlsl -entry entryManufacture -specialize rt::MissInput<MissContext> -no-codegen

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

// Ordinary instantiations of the same bodies produce no diagnostics: the walk substitutes,
// classifies, and moves on. This file's exhaustive mode enforces the silence.
extension ::Payload : IRead
{
    uint read() { return value; }
}

struct PlainCarrier : ICarrier
{
    typealias Item = ::Payload;
};

[shader("compute")]
void entryManufacture<T>()
{
    T t = T();
//SPECIALIZE: fresh construction of a structural ray-tracing stage input
}

// Construction in return position reaches the same rule as the local-copy spelling.
T makeValue<T>()
{
    return T();
//CHECK:    ^ fresh construction of a structural ray-tracing stage input
//CHECK:    ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
}

// A recursive generic is legal: the abstract body check completes the inner call while the
// body is still being checked, and the walk must not re-enter it (that would be reported as
// a cyclic reference). The construction in the body is still diagnosed once.
uint recurseMake<T>(int depth)
{
    if (depth <= 0)
        return 1;
    T fresh = T();
//CHECK:       ^ fresh construction of a structural ray-tracing stage input
//CHECK:       ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
    return recurseMake<T>(depth - 1);
}

// Mutual recursion with an ordinary argument stays silent, and must not be a cyclic
// reference either.
uint ping<T>(int depth);
uint pong<T>(int depth)
{
    if (depth <= 0)
        return 0;
    return ping<T>(depth - 1);
}
uint ping<T>(int depth)
{
    if (depth <= 0)
        return 1;
    return pong<T>(depth - 1);
}

// A recursive call supplying concrete stage-input arguments must not escape: its completion
// happens mid-check and is skipped, and the outer walk re-enters it through the dedup reset.
uint userConcrete<T>(int depth)
{
    if (depth <= 0)
    {
        T fresh = T();
//CHECK:           ^ fresh construction of a structural ray-tracing stage input
//CHECK:           ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
        return 1;
    }
    return userConcrete<rt::MissInput<MissContext>>(depth - 1);
//CHECK:   ^^^^^^^^^^^^ in the instantiation of 'userConcrete'
}

// Grouped declaration statements reach the same rules: the walker descends into DeclGroups.
uint multi<T>()
{
    T a = T(), b = T();
//CHECK:   ^ fresh construction of a structural ray-tracing stage input
//CHECK:   ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
//CHECK:            ^ fresh construction of a structural ray-tracing stage input
//CHECK:            ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
    T c, d;
/*CHECK:
      ^ storage of a structural ray-tracing stage input
      ^ ray-tracing stage input type 'rt.MissInput<MissContext>' may only be used as an 'in' parameter or an initialized local copy
         ^ storage of a structural ray-tracing stage input
         ^ ray-tracing stage input type 'rt.MissInput<MissContext>' may only be used as an 'in' parameter or an initialized local copy
*/
    return 1;
}

// The rejected storage positions of the shared predicate, through the walk path.
uint rejectVariants<T>()
{
    static T s = T();
//CHECK:     ^ storage of a structural ray-tracing stage input
//CHECK:     ^ ray-tracing stage input type 'rt.MissInput<MissContext>' may only be used as an 'in' parameter or an initialized local copy
//CHECK:          ^ fresh construction of a structural ray-tracing stage input
//CHECK:          ^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be constructed here; it must come from the current invocation
    T outParam;
/*CHECK:
      ^^^^^^^^ storage of a structural ray-tracing stage input
      ^^^^^^^^ ray-tracing stage input type 'rt.MissInput<MissContext>' may only be used as an 'in' parameter or an initialized local copy
*/
    T uniformParam;
/*CHECK:
      ^^^^^^^^^^^^ storage of a structural ray-tracing stage input
      ^^^^^^^^^^^^ ray-tracing stage input type 'rt.MissInput<MissContext>' may only be used as an 'in' parameter or an initialized local copy
*/
    return 1;
}

// A generic whose local becomes the stage interface itself reports the runtime-value error,
// the same message the concrete spelling produces.
uint probe<T>()
{
    T stage;
/*CHECK:
      ^^^^^ runtime use of a structural ray-tracing stage type
      ^^^^^ structural ray-tracing stage type 'rt.IMissShader' has no runtime representation
*/
    return 1;
}

// A struct whose field names its own generic form with a growing argument classifies within
// the depth budget instead of exhausting the stack.
struct GrowA<T>
{
    Ptr<GrowA<T[]>> next;
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

        // Ordinary instantiations stay silent here by construction (exhaustive mode).
        input.payload.value += manufacture<PlainCarrier>();
        let made = makeValue<rt::MissInput<Context>>();
//CHECK:           ^^^^^^^^^ in the instantiation of 'makeValue'
        passInput(made);
        input.payload.value += recurseMake<rt::MissInput<Context>>(2);
//CHECK:                       ^^^^^^^^^^^ in the instantiation of 'recurseMake'
        input.payload.value += ping<uint>(4);
        input.payload.value += userConcrete<uint>(2);
        input.payload.value += probe<rt::IMissShader>();
//CHECK:                       ^^^^^ in the instantiation of 'probe'
        input.payload.value += multi<rt::MissInput<Context>>();
//CHECK:                       ^^^^^ in the instantiation of 'multi'
        input.payload.value += rejectVariants<rt::MissInput<Context>>();
//CHECK:                       ^^^^^^^^^^^^^^ in the instantiation of 'rejectVariants'

        // The same instantiation completed twice reports once: the exhaustive mode fails on
        // any duplicate diagnostic this second call would emit.
        makeFresh<rt::MissInput<Context>>();

        // A growing generic struct classifies within the depth budget and stays silent.
        GrowA<int> grow;

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
