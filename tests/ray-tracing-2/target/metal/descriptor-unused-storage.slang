//TEST:SIMPLE(filecheck=PRE): -experimental-feature -entry main -stage compute -target metal -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage compute -target metal -dump-ir-after prepareMetalStructuralRayTracing
//TEST:SIMPLE(filecheck=MODULE): -experimental-feature -entry main -stage compute -target slang-vm -o descriptor-unused-storage.slang-module

// MODULE: note[E00088]: the Slang module file format is not yet finalized

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<uint> output;

[noinline]
uint retainDescriptorType(rt::TraceProgramDescriptor<Schema> value, uint marker)
{
    // Keep the call observable without reading the descriptor's opaque implementation fields.
    // The descriptor still has to cross the helper parameter and call-argument edges.
    return marker + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // No trace operation activates this schema, so there are no executable Metal table
    // signatures to synthesize. The compiler retains the opaque bound-buffer handle and consumes
    // the schema-carrying resource type before ordinary Metal legalization.
    output[0] = retainDescriptorType(program, 0);
}

// Prove that this test actually starts with the compiler-owned resource and retains a descriptor-
// bearing helper. Otherwise a post-pass `-NOT` check could pass without exercising the lowering.
// PRE: result code = 0
// PRE: TraceProgramDescriptor
// PRE: func %retainDescriptorType
// PRE: call %retainDescriptorType

// Descriptor-only Metal preparation must lower every opaque resource while preserving the helper and
// its observable call. The leading and trailing checks cover the complete IR dump.
// IR: result code = 0
// IR-NOT: TraceProgramDescriptor(
// IR: func %retainDescriptorType
// IR-NOT: TraceProgramDescriptor(
// IR: call %retainDescriptorType
// IR-NOT: TraceProgramDescriptor(
