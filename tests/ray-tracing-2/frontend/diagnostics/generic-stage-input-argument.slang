//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -entry main -stage compute -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -entry main -stage compute -target metal -no-codegen

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestRecord
{
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TestTraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

void consume<T>(T value)
{
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    static void invoke(rt::ClosestHitInput<HitContext> input)
    {
        consume(input);
//CHECK:^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
//CHECK:^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' cannot be used as a generic type argument
    }
}

interface IReadable
{
    uint read();
}

extension rt::ClosestHitInput<HitContext> : IReadable
{
    [require(closesthit)]
    uint read() { return payload.value; }
}

uint makeAndRead<T>() where T : IReadable
{
    return T().read();
}

struct Wrapper<T>
{
}

// Even a generic type with no fields cannot use a stage or stage-input type argument.
// Cover direct types, type aliases, and resource wrappers.
struct Phantom<T>
{
}

typealias HitInputAlias = rt::ClosestHitInput<HitContext>;

typealias InvalidPhantom = Phantom<rt::ClosestHitInput<HitContext>>;
//CHECK:                               ^^^^^^^^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
//CHECK:                               ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' cannot be used as a generic type argument

typealias InvalidAliasedPhantom = Phantom<HitInputAlias>;
//CHECK:                                  ^^^^^^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
//CHECK:                                  ^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' cannot be used as a generic type argument

typealias InvalidStagePhantom = Phantom<ClosestHit>;
//CHECK:                                ^^^^^^^^^^ runtime use of a structural ray-tracing stage type
//CHECK:                                ^^^^^^^^^^ structural ray-tracing stage type 'ClosestHit' has no runtime representation

ConstantBuffer<HitInputAlias> invalidStageInputBuffer;
//CHECK:       ^^^^^^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
//CHECK:       ^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' cannot be used as a generic type argument

// Metadata types remain ordinary generic arguments; only shader stages and inputs retain
// the restriction needed to prevent a generic body from constructing or storing those values.
typealias LegalMetadataArgument = Phantom<rt::ITraceProgramSchema>;

typealias LegalTracer = rt::RayTracer<TestSchema>;
typealias LegalDescriptor = rt::TraceProgramDescriptor<TestSchema>;

// Shader lists accept stages as type arguments without constructing their values.
struct Miss : rt::IMissShader
{
    typealias Context = HitContext;
    static void invoke(rt::MissInput<Context> input) {}
}

typealias LegalMissList = rt::MissShaderList<Miss>;

void useType<T>() {}
void useTypes<each T>() {}

void materialize<T>()
{
    T value = T();
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = makeAndRead<rt::ClosestHitInput<HitContext>>();
//CHECK:        ^^^^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
//CHECK:        ^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' cannot be used as a generic type argument

    useType<Wrapper<rt::ClosestHitInput<HitContext>>>();
//CHECK:                ^^^^^^^^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
//CHECK:                ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' cannot be used as a generic type argument

    useType<ClosestHit>();
/*CHECK:
    ^^^^^^^ runtime use of a structural ray-tracing stage type
    ^^^^^^^ structural ray-tracing stage type 'ClosestHit' has no runtime representation
*/

    // Variadic substitutions arrive at semantic checking as a single concrete type pack. Its
    // immediate elements obey the same generic-argument restriction as individual types.
    useTypes<rt::ClosestHitInput<HitContext>>();
/*CHECK:
    ^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
    ^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' cannot be used as a generic type argument
*/

    useTypes<ClosestHit>();
/*CHECK:
    ^^^^^^^^ runtime use of a structural ray-tracing stage type
    ^^^^^^^^ structural ray-tracing stage type 'ClosestHit' has no runtime representation
*/

    materialize<TestSchema>();
}
