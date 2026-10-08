//DIAGNOSTIC_TEST:SIMPLE(filecheck=RESOURCES): -experimental-feature -entry main -stage compute -target hlsl -DTEST_RESOURCES
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CALL): -experimental-feature -entry main -stage compute -target hlsl -DTEST_CALL
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ARGUMENT): -experimental-feature -entry main -stage compute -target hlsl -DTEST_ARGUMENT
//DIAGNOSTIC_TEST:SIMPLE(filecheck=INITIALIZER): -experimental-feature -entry main -stage compute -target hlsl -DTEST_INITIALIZER
//DIAGNOSTIC_TEST:SIMPLE(filecheck=OPTIONAL): -experimental-feature -entry main -stage compute -target hlsl -DTEST_OPTIONAL

import slang.raytracing;

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

// A descriptor may be bound directly or as part of a parameter block.
rt::TraceProgramDescriptor<Schema> descriptor;

struct Bindings
{
    rt::TraceProgramDescriptor<Schema> descriptor;
}
ParameterBlock<Bindings> bindings;

#ifdef TEST_OPTIONAL
// Optional<T> needs an ordinary default value for its absent case, which an opaque descriptor does
// not provide.
Optional<rt::TraceProgramDescriptor<Schema>> optionalDescriptor;
// OPTIONAL: 'Optional<T>' cannot wrap a resource or opaque type
// OPTIONAL: 'rt.TraceProgramDescriptor<Schema>' is a resource or opaque type and cannot be used as the value type of 'Optional<T>'.
#endif

[numthreads(1, 1, 1)]
void main()
{
#ifdef TEST_RESOURCES
    descriptor.resources;
// RESOURCES: 'resources' is not a member of 'rt.TraceProgramDescriptor<Schema>'
#elif defined(TEST_CALL)
    rt::TraceProgramDescriptor<Schema> fromCall = rt::TraceProgramDescriptor<Schema>();
//CALL: not enough arguments to call
//CALL: not enough arguments to call (got 0, expected 1)
//CALL: candidate: private rt::TraceProgramDescriptor<Schema>.init(int)
#elif defined(TEST_ARGUMENT)
    rt::TraceProgramDescriptor<Schema> fromArgument = rt::TraceProgramDescriptor<Schema>(0);
//ARGUMENT: declaration not accessible
//ARGUMENT: 'TraceProgramDescriptor.init' is not accessible from the current context.
//ARGUMENT: type mismatch in expression
//ARGUMENT: expected an expression of type 'rt.TraceProgramDescriptor<Schema>', got 'int'
#elif defined(TEST_INITIALIZER)
    rt::TraceProgramDescriptor<Schema> fromInitializer = {};
//INITIALIZER: not enough arguments to call
//INITIALIZER: not enough arguments to call (got 0, expected 1)
//INITIALIZER: candidate: private rt::TraceProgramDescriptor<Schema>.init(int)
#endif
}
