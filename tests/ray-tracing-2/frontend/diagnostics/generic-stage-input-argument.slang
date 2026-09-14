//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -entry main -stage compute -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -entry main -stage compute -target metal

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

void consume<T>(T value)
{
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        consume(input);
//CHECK:^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter
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

// A generic type may erase its argument physically, but passing a compiler-provided stage input or
// a structural stage as a type argument is still invalid. Cover direct, type-aliased, and
// resource-wrapper applications.
struct Phantom<T>
{
}

typealias HitInputAlias = rt::ClosestHitInput<HitContext>;

typealias InvalidPhantom = Phantom<rt::ClosestHitInput<HitContext>>;
//CHECK:                               ^^^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:                               ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter

typealias InvalidAliasedPhantom = Phantom<HitInputAlias>;
//CHECK:                                  ^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                  ^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter

typealias InvalidStagePhantom = Phantom<ClosestHit>;
//CHECK:                                ^^^^^^^^^^ runtime use of a structural ray-tracing stage type
//CHECK:                                ^^^^^^^^^^ structural ray-tracing stage type 'ClosestHit' has no runtime representation

ConstantBuffer<HitInputAlias> invalidStageInputBuffer;
//CHECK:       ^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:       ^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter

// Structural metadata is compile-time schema information, so it remains a valid generic argument.
typealias LegalMetadataArgument = Phantom<rt::ITraceProgramSchema>;

// Open-section tags and listed stages are themselves generic arguments, but the resulting type is
// compiler-recognized structural metadata rather than an ordinary user container.
interface IOpenMissTag : rt::IMissShader {}

typealias LegalOpenSection = rt::OpenMissShaders<IOpenMissTag>;

void useType<T>() {}
void useTypes<each T>() {}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = makeAndRead<rt::ClosestHitInput<HitContext>>();
//CHECK:        ^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:        ^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter

    useType<Wrapper<rt::ClosestHitInput<HitContext>>>();
//CHECK:                ^^^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:                ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter

    useType<ClosestHit>();
/*CHECK:
    ^^^^^^^ runtime use of a structural ray-tracing stage type
    ^^^^^^^ structural ray-tracing stage type 'ClosestHit' has no runtime representation
*/

    // Variadic substitutions arrive at semantic checking as a single concrete type pack. Its
    // immediate elements obey the same no-runtime-representation rule as ordinary arguments.
    useTypes<rt::ClosestHitInput<HitContext>>();
/*CHECK:
    ^^^^^^^^ storage of a structural ray-tracing stage input
    ^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter
*/

    useTypes<ClosestHit>();
/*CHECK:
    ^^^^^^^^ runtime use of a structural ray-tracing stage type
    ^^^^^^^^ structural ray-tracing stage type 'ClosestHit' has no runtime representation
*/
}
