// direct-spirv-emit.slang

//TEST:SIMPLE:-target spirv -entry computeMain -stage compute -emit-spirv-directly

// Test ability to directly output SPIR-V

[numthreads(4,1,1)]
void computeMain()
{}
