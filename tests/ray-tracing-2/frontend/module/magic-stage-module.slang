//TEST:SIMPLE: -experimental-feature -entry FirstMiss<Payload> -no-codegen
//TEST:SIMPLE: -experimental-feature -entry SecondMiss -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/magic_stage_library.slang -experimental-feature -o tests/ray-tracing-2/frontend/module/magic-first-stage-cache.slang-module
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/magic_stage_secondary_library.slang -experimental-feature -o tests/ray-tracing-2/frontend/module/magic-second-stage-cache.slang-module
//TEST:SIMPLE(filecheck=IR): -experimental-feature -r tests/ray-tracing-2/frontend/module/magic-first-stage-cache.slang-module -r tests/ray-tracing-2/frontend/module/magic-second-stage-cache.slang-module -entry FirstMiss<Payload> -dump-ir -no-codegen
//TEST:SIMPLE: -experimental-feature -r tests/ray-tracing-2/frontend/module/magic-first-stage-cache.slang-module -r tests/ray-tracing-2/frontend/module/magic-second-stage-cache.slang-module -entry SecondMiss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CODEGEN): -experimental-feature -r tests/ray-tracing-2/frontend/module/magic-first-stage-cache.slang-module -r tests/ray-tracing-2/frontend/module/magic-second-stage-cache.slang-module -entry FirstMiss<Payload> -target hlsl

import slang.raytracing;
import magic_stage_library;
import magic_stage_secondary_library;

// The serialized filenames deliberately differ from the import names, so the first two runs
// always use source. Both application interfaces inherit the standard rt::IMissShader contract;
// their ordinary inheritance witnesses must preserve its stage identity after serialization.
// IR-DAG: let %firstx5FIBackgroundStage{{.*}}= interface(
// IR-DAG: let %secondx5FIBackgroundStage{{.*}}= interface(
// IR-DAG: let %rtx5FIMissShader{{.*}}= interface(
// IR-DAG: witness_table_entry(%firstx5FIBackgroundStagex5Fx24inheritance,
// IR-DAG: witness_table_entry(%rtx5FIMissShaderx5Finvoke,specialize(

struct Payload
{
    uint value;
}

struct Traversal : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct FirstContext<P> : rt::IPayloadContext
{
    typealias TraceContext = Traversal;
    typealias Record = void;
    typealias Payload = P;
}

// The selected invoke witness must retain P = Payload through the imported refinement of
// rt::IMissShader. Both source and serialized application interfaces must select this specialization.
struct FirstMiss<P> : first::IBackgroundStage
{
    typealias Context = FirstContext<P>;
    static void invoke(in rt::MissInput<Context> input) {}
/*CODEGEN:
                ^^^^^^ structural ray-tracing entry point was not lowered
                ^^^^^^ structural ray-tracing entry point 'FirstMiss.invoke' must be converted to a native adapter before target code generation
*/
}

struct SecondContext : rt::IPayloadContext
{
    typealias TraceContext = Traversal;
    typealias Record = uint;
    typealias Payload = uint2;
}

struct SecondMiss : second::IBackgroundStage
{
    typealias Context = SecondContext;
    static void invoke(in rt::MissInput<Context> input) {}
}
