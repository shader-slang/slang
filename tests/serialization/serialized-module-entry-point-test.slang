// serialized-module-entry-point-test.slang

//TEST:COMPILE: -target hlsl -profile cs_5_0 -entry computeMain tests/serialization/serialized-module-entry-point.slang -o tests/serialization/serialized-module-entry-point.slang-module
//TEST:COMPILE: tests/serialization/serialized-module.slang -o tests/serialization/serialized-module.slang-module
//TEST:COMPARE_COMPUTE_EX:-slang -compute -xslang -r -xslang tests/serialization/serialized-module-entry-point.slang-module -xslang -r -xslang tests/serialization/serialized-module.slang-module

//TEST_INPUT:ubuffer(data=[0 0 0 0 ], stride=4):dxbinding(0),glbinding(0),out,name outputBuffer

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID);