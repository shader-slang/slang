//TEST:SIMPLE: -experimental-feature -entry FirstMiss<Payload> -no-codegen
//TEST:SIMPLE: -experimental-feature -entry SecondMiss -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/magic_stage_library.slang -experimental-feature -o tests/ray-tracing-2/frontend/module/magic-first-stage-cache.slang-module
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/magic_stage_secondary_library.slang -experimental-feature -o tests/ray-tracing-2/frontend/module/magic-second-stage-cache.slang-module
//TEST:SIMPLE(filecheck=IR): -experimental-feature -r tests/ray-tracing-2/frontend/module/magic-first-stage-cache.slang-module -r tests/ray-tracing-2/frontend/module/magic-second-stage-cache.slang-module -entry FirstMiss<Payload> -dump-ir -no-codegen
//TEST:SIMPLE: -experimental-feature -r tests/ray-tracing-2/frontend/module/magic-first-stage-cache.slang-module -r tests/ray-tracing-2/frontend/module/magic-second-stage-cache.slang-module -entry SecondMiss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CODEGEN): -experimental-feature -r tests/ray-tracing-2/frontend/module/magic-first-stage-cache.slang-module -r tests/ray-tracing-2/frontend/module/magic-second-stage-cache.slang-module -entry FirstMiss<Payload> -target hlsl

import magic_stage_library;
import magic_stage_secondary_library;

// The serialized filenames deliberately differ from the import names, so the first two runs
// always use source. Each serialized module retains its own stage-interface instruction. Loading
// another contract with the same magic type must neither replace it nor erase its stage identity.
// IR-DAG: [nameHint("first.IBackgroundStage")]
// IR-DAG: [nameHint("second.IBackgroundStage")]
// IR-DAG: miss_stage_interface(
// IR-DAG: miss_stage_interface(

struct Payload
{
    uint value;
}

struct FirstTraversal : first::ITraversal
{
    typealias AccelerationStructure = uint;
    typealias Motion = void;
}

struct FirstContext<P> : first::IPayloadContext
{
    typealias TraceContext = FirstTraversal;
    typealias Record = void;
    typealias Payload = P;
}

// The selected invoke witness must retain P = Payload. Both source and serialized contracts
// must select this specialization, even after another module declares its own MissShaderType.
struct FirstMiss<P> : first::IBackgroundStage
{
    typealias Context = FirstContext<P>;
    void invoke(in first::BackgroundInput<Context> input) {}
}

struct SecondTraversal : second::ITraversal
{
    typealias AccelerationStructure = uint;
    typealias Motion = void;
}

struct SecondContext : second::IPayloadContext
{
    typealias TraceContext = SecondTraversal;
    typealias Record = uint;
    typealias Payload = uint2;
}

struct SecondMiss : second::IBackgroundStage
{
    typealias Context = SecondContext;
    void invoke(in second::BackgroundInput<Context> input) {}
}

// CODEGEN: structural ray-tracing entry point 'FirstMiss.invoke' must be converted to a native adapter before target code generation
