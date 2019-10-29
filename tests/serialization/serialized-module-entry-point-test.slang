// serialized-module-entry-point-test.slang

//TEST:SIMPLE_EX: tests/serialization/serialized-module-entry-point.slang -o tests/serialization/serialized-module-entry-point.slang-module
//TEST:COMPARE_COMPUTE_EX:-slang -compute -xslang -r -xslang tests/serialization/serialized-module-entry-point.slang-module

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID);