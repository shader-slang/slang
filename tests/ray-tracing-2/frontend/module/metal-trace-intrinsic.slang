//TEST:SIMPLE(filecheck=IR): -target metal -dump-ir -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=REACHABLE): -target metal -entry traceMain -stage compute
//TEST:SIMPLE(filecheck=UNUSED): -target metal -entry unusedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=REACHABLE): -target metal -entry selectedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SELECTED): -target hlsl -entry selectedMain -stage compute
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/metal_trace_intrinsic_library.slang -o tests/ray-tracing-2/frontend/module/metal-trace-intrinsic-cache.slang-module
//TEST:SIMPLE(filecheck=IR): -r tests/ray-tracing-2/frontend/module/metal-trace-intrinsic-cache.slang-module -target metal -dump-ir -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=REACHABLE): -r tests/ray-tracing-2/frontend/module/metal-trace-intrinsic-cache.slang-module -target metal -entry traceMain -stage compute
//TEST:SIMPLE(filecheck=UNUSED): -r tests/ray-tracing-2/frontend/module/metal-trace-intrinsic-cache.slang-module -target metal -entry unusedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=REACHABLE): -r tests/ray-tracing-2/frontend/module/metal-trace-intrinsic-cache.slang-module -target metal -entry selectedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SELECTED): -r tests/ray-tracing-2/frontend/module/metal-trace-intrinsic-cache.slang-module -target hlsl -entry selectedMain -stage compute

// The cache name differs from the import name so the first runs always use the source module.
// Both imported declarations must produce one opcode, retaining the typed descriptor and mutable
// payload. Zero is still a motion time: the no-motion call omits that operand altogether.
import metal_trace_intrinsic_library;

struct Schema : traceir::IProgramShape {}

struct Payload
{
    uint value;
}

RWStructuredBuffer<uint> output;

// IR-LABEL: func %traceMain
// IR: param %program : TraceProgramDescriptor(%Schema, %{{[0-9]+}})
// IR: [[PAYLOAD:%[A-Za-z0-9_]+]] : Ptr(%Payload) = var
// IR: [[ORIGIN:%[0-9]+]] : Vec(Float, 3 : Int) = makeVector(1 : Float, 2 : Float, 3 : Float)
// IR: [[DIRECTION:%[0-9]+]] : Vec(Float, 3 : Int) = makeVector(0 : Float, 1 : Float, 0 : Float)
// IR: metalTraceRay(%scene, %program, 1 : UInt, 2 : UInt, 3 : UInt, 4 : UInt, 5 : UInt, [[ORIGIN]], 0.25 : Float, [[DIRECTION]], 100 : Float, [[PAYLOAD]])
// IR: [[MOTION_ORIGIN:%[0-9]+]] : Vec(Float, 3 : Int) = makeVector(4 : Float, 5 : Float, 6 : Float)
// IR: [[MOTION_DIRECTION:%[0-9]+]] : Vec(Float, 3 : Int) = makeVector(0 : Float, 0 : Float, 1 : Float)
// IR: metalTraceRay(%scene, %program, 6 : UInt, 7 : UInt, 8 : UInt, 9 : UInt, 10 : UInt, [[MOTION_ORIGIN]], 0.5 : Float, [[MOTION_DIRECTION]], 200 : Float, [[PAYLOAD]], 0 : Float)
[numthreads(1, 1, 1)]
void traceMain(
    uniform RaytracingAccelerationStructure scene,
    uniform traceir::ProgramHandle<Schema> program)
{
    Payload payload = { 11 };
    traceir::trace(scene, program, 1, 2, 3, 4, 5,
        float3(1, 2, 3), 0.25f, float3(0, 1, 0), 100.0f, payload);
    traceir::traceMotion(scene, program, 6, 7, 8, 9, 10,
        float3(4, 5, 6), 0.5f, float3(0, 0, 1), 200.0f, payload, 0.0f);
    output[0] = payload.value;
}

// This entry has no structural entry marker, and the intrinsic declarations have no source
// operation marker. The unfinished instruction itself must trigger the frontend boundary check.
// REACHABLE: structural ray-tracing operation 'trace' reachable from this entry point must be lowered

// An unused function containing the same instruction must not block unrelated Metal compilation.
// UNUSED-NOT: metalTraceRay
// UNUSED: void unusedMain
// UNUSED-NOT: metalTraceRay
[numthreads(1, 1, 1)]
void unusedMain()
{
    output[0] = 17;
}

// The linker's target selection must remove the Metal operation before the boundary check runs.
// The bound descriptor still needs lowering even when the selected branch does not read it.
// SELECTED-NOT: structural ray-tracing operation 'trace'
// SELECTED: structural ray-tracing program descriptor must be lowered before target code generation
// SELECTED-NOT: structural ray-tracing operation 'trace'
[numthreads(1, 1, 1)]
void selectedMain(
    uniform RaytracingAccelerationStructure scene,
    uniform traceir::ProgramHandle<Schema> program)
{
    __target_switch
    {
    case metal:
        Payload payload = { 19 };
        traceir::trace(scene, program, 1, 2, 3, 4, 5,
            float3(1, 2, 3), 0.25f, float3(0, 1, 0), 100.0f, payload);
        output[0] = payload.value;
        break;
    default:
        output[0] = 23;
        break;
    }
}
