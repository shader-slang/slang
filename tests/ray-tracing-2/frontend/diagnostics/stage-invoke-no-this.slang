//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import slang.raytracing.portable;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;

    // The stage entry is static, so it cannot call an instance helper through this.
    static void invoke(in rt::MissInput<Context> input)
    {
        this.helper();
//CHECK:     ^^^^^^ static function cannot refer to non-static member via `this`
//CHECK:     ^^^^^^ static function cannot refer to non-static member `helper` via `this`
    }

    void helper() {}
}
