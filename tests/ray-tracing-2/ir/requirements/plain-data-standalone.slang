//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry BadClosestHit -stage closesthit -target hlsl -profile sm_6_5

import slang.raytracing;

struct InvalidPayload
/*CHECK:
       ^^^^^^^^^^^^^^ structural ray-tracing ABI data is not portable plain data
       ^^^^^^^^^^^^^^ type 'InvalidPayload' is used as structural ray-tracing payload data but does not have a fixed-size, copyable plain-data representation
*/
{
    Texture2D texture;
}

struct InvalidRecord
/*CHECK:
       ^^^^^^^^^^^^^ structural ray-tracing ABI data is not portable plain data
       ^^^^^^^^^^^^^ type 'InvalidRecord' is used as structural ray-tracing record data but does not have a fixed-size, copyable plain-data representation
*/
{
    uint* pointer;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = InvalidPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = InvalidRecord;
}

struct BadClosestHit : rt::IClosestHitShader
{
    typealias Context = TestHitContext;
    void invoke(rt::ClosestHitInput<Context> input) {}
}
