//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

import slang.raytracing;

[raypayload]
struct InvalidPayload
{
    float missingAccess;
//CHECK:  ^^^^^^^^^^^^^ ray payload field missing access qualifiers
//CHECK:  ^^^^^^^^^^^^^ field 'missingAccess' in ray payload struct must have either 'read' OR 'write' access qualifiers
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct PayloadContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = InvalidPayload;
    typealias Record = void;
}
