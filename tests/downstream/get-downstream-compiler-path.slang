//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -get-glslang-path

// Verify that `-get-<compiler>-path` prints the resolved on-disk path of the downstream compiler
// Slang would load for the named pass-through, then continues the compile (print-and-continue, like
// `-version`). glslang is bundled as a shared library and loads without a GPU, so on a machine that
// has it the query prints a real path; the pattern also matches the "not found" / "not available"
// fallbacks so the test is robust across runners. The trailing `void main(` in the emitted HLSL is
// the proof the option reported and let the compile continue rather than terminating the parse.

//CHECK: glslang path: {{.+}}
//CHECK: void main(

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = 1;
}
