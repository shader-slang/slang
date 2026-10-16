//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -ignore-capabilities -entry main -stage compute -target hlsl
//TEST:SIMPLE: -experimental-feature -entry FieldMiss -entry ExtensionMiss -target hlsl -no-codegen

// Both stage structs retain ordinary fields in generated code. Their copies, constructors and
// generic helpers can supply values to compute code without calling the static stage method.
// CHECK: struct ExtensionMiss_
// CHECK: uint state_
// CHECK: struct FieldMiss_
// CHECK: uint state_
// CHECK: void main()
// CHECK: output_{{[0-9]+}}[int(0)] = FieldMiss_read_
// CHECK: output_{{[0-9]+}}[int(1)] = {{.*}}.state_{{[0-9]+}} + {{.*}}.state_

import slang.raytracing;

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

struct FieldMiss : rt::IMissShader
{
    typealias Context = MissContext;
    uint state;

    __init() { state = 0; }
    __init(uint value) { state = value; }
    uint read() { return state; }

    static void invoke(in rt::MissInput<Context> input)
    {
        // A static method can explicitly create and use an ordinary object. Dispatch does
        // not create this object automatically or supply its fields to the method.
        FieldMiss local = FieldMiss(3);
        input.payload += local.read();
    }
}

struct ExtensionMiss
{
    uint state;
}

extension ExtensionMiss : rt::IMissShader
{
    typealias Context = MissContext;
    static void invoke(in rt::MissInput<Context> input)
    {
        FieldMiss local = FieldMiss(7);
        // Both calls pass only input. The object in local.invoke does not supply a hidden this.
        local.invoke(input);
        FieldMiss::invoke(input);
    }
}

struct Box<T>
{
    T value;
}

struct Phantom<T> {}
typealias Stages = FieldMiss[1];

T identity<T>(T value) { return value; }
T make<T>() { return T(); }
T stageIdentity<T : rt::IMissShader>(T value) { return value; }
void useTypes<each T>() {}

FieldMiss makeStage()
{
    FieldMiss original = FieldMiss(1);
    FieldMiss copy = original;
    copy.state = 2;
    return copy;
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    FieldMiss local = makeStage();
    FieldMiss genericCopy = stageIdentity(identity(local));
    FieldMiss defaultValue = make<FieldMiss>();
    Box<Box<FieldMiss>> nested = {{genericCopy}};
    Optional<FieldMiss> optionalValue = local;
    Stages stages = {local};
    Stages();
    Phantom<FieldMiss> tag = {};
    useTypes<FieldMiss, ExtensionMiss>();

    ExtensionMiss extended = {4};
    ExtensionMiss extendedCopy = stageIdentity(extended);
    output[0] = nested.value.value.read() + defaultValue.read();
    output[1] = optionalValue.value.state + extendedCopy.state + stages[0].read();
}
