//TEST:SIMPLE(filecheck=CHECK_GLSL):-stage compute -entry computeMain -target glsl -line-directive-mode none
// We can't enable because output source includes path to prelude.
//DISABLE_TEST:SIMPLE:-stage compute -entry computeMain -target cpp -line-directive-mode none
//TEST:SIMPLE(filecheck=CHECK_CUDA):-stage compute -entry computeMain -target cuda -line-directive-mode none
// Not currently supported on HLSL
//DISABLE_TEST:SIMPLE:-stage compute -entry computeMain -target hlsl -line-directive-mode none
// With `slang-llvm` this will crash, but the call stack isn't really usable.
// With downstream host compilers this should break into the debugger. It's not enabled as that isn't testable.
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj -Xslang... -O0 -g -X.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer

// CHECK_GLSL: spirv_instruction(id = 1, set = "NonSemantic.DebugBreak")
// CHECK_GLSL: glslDebugBreak{{.*}}();
// CHECK_GLSL: void main
// CHECK_GLSL: glslDebugBreak{{.*}}();

// CHECK_CUDA:__brkpt()
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = int(dispatchThreadID.x);

    debugBreak();

    outputBuffer[idx] = idx;    
}
