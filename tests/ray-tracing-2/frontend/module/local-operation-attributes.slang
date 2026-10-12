//DIAGNOSTIC_TEST:SIMPLE(filecheck=CALL): -entry callableMain -stage raygeneration -target hlsl
//TEST:COMPILE: tests/ray-tracing-2/frontend/module/local_operation_library.slang -o tests/ray-tracing-2/frontend/module/local-operation-cache.slang-module
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CALL): -r tests/ray-tracing-2/frontend/module/local-operation-cache.slang-module -entry callableMain -stage raygeneration -target hlsl

// A different serialized filename keeps the first check on the source import path.
// Module-local attribute syntax must identify callable dispatch before and after serialization.
import local_operation_library;

void callableMain()
{
    callShader();
}

// CALL: structural ray-tracing operation 'callShader' reachable from this entry point must be lowered
