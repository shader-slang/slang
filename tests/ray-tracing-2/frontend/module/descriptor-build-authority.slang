//TEST:SIMPLE(filecheck=CHECK): -compile-slang-raytracing-module -entry main -stage compute -dump-ir -no-codegen

module raytracing;

namespace rt
{
public interface ITraceProgramSchema {}

// Only the standard-module build is allowed to establish the descriptor's AST and IR identities.
__magic_type(TraceProgramDescriptorType)
__intrinsic_type(TraceProgramDescriptor)
public struct TraceProgramDescriptor<Schema>
    where Schema : ITraceProgramSchema
{
    private __init(int unavailable) {}
}
}

struct Schema : rt::ITraceProgramSchema {}

rt::TraceProgramDescriptor<Schema> descriptor;

// CHECK: TraceProgramDescriptor(

[numthreads(1, 1, 1)]
void main() {}
