// A push-constant block that only an `export`ed function reads is still
// declared in GLSL output: the function is emitted because it is exported, even
// though no entry point calls it. A lone push-constant block that nothing reads
// is also kept under `-preserve-params`, because only a second block makes
// glslang reject the output (shader-slang/slang#13713).

//TEST:SIMPLE(filecheck=EXPORT): -target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=EXPORT): -target spirv -emit-spirv-via-glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=LONE): -target glsl -entry main -stage compute -preserve-params -DNO_EXPORT

struct PC
{
    float4 a;
}

[[vk::push_constant]]
uniform PC push;

RWStructuredBuffer<float> outBuf;

#ifndef NO_EXPORT
export float helper()
{
    return push.a.x;
}
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = 1;
}

// EXPORT-NOT: error[
// EXPORT: {{layout\(push_constant\)|OpEntryPoint}}

// LONE-NOT: error[
// LONE: layout(push_constant)
// LONE-NEXT: uniform block_PC_
