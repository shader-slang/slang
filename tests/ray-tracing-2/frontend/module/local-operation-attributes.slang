//DIAGNOSTIC_TEST:SIMPLE(filecheck=TRACE): -entry traceMain -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=PAYLOAD): -entry tracePayloadMain -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CALL): -entry callableMain -stage raygeneration -target hlsl
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/local_operation_library.slang -o tests/ray-tracing-2/frontend/module/local-operation-cache.slang-module
//DIAGNOSTIC_TEST:SIMPLE(filecheck=TRACE): -r tests/ray-tracing-2/frontend/module/local-operation-cache.slang-module -entry traceMain -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=PAYLOAD): -r tests/ray-tracing-2/frontend/module/local-operation-cache.slang-module -entry tracePayloadMain -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CALL): -r tests/ray-tracing-2/frontend/module/local-operation-cache.slang-module -entry callableMain -stage raygeneration -target hlsl

// A different serialized filename keeps the first three checks on the source import path.
// Module-local attribute syntax must identify each operation before and after serialization.
import local_operation_library;

void traceMain()
{
    traceWithoutPayload();
}

void tracePayloadMain()
{
    uint payload = 0;
    traceWithPayload(payload);
}

void callableMain()
{
    callShader();
}

// TRACE: structural ray-tracing operation 'trace with implicit empty payload' reachable from this entry point must be lowered
// PAYLOAD: structural ray-tracing operation 'trace with explicit payload' reachable from this entry point must be lowered
// CALL: structural ray-tracing operation 'callShader' reachable from this entry point must be lowered
