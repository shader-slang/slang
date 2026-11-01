// After E39033 on GLSL, the GLSL is not passed on to glslang, so glslang's own
// push-constant error does not appear (shader-slang/slang#13713).

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl -entry main -stage compute

struct PC { float4 a; }

[[vk::push_constant]]
uniform PC push;

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float k) { outBuf[0] = push.a.x + k; }

// CHECK: E39033
// CHECK-NOT: glslang
