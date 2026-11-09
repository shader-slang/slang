// Under `-preserve-params`, a module has two push-constant blocks. Only an
// `export`ed function reads `push`, and nothing reads `push2`. `push` must stay
// declared, because the exported function is emitted and uses it, while
// `push2` is omitted, so glslang sees one block (shader-slang/slang#13713).

//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry main -stage compute -preserve-params
//TEST:SIMPLE(filecheck=VIA): -target spirv-asm -emit-spirv-via-glsl -entry main -stage compute -preserve-params

struct PC { float4 a; }
struct PC2 { float4 b; }

[[vk::push_constant]]
uniform PC push;

[[vk::push_constant]]
uniform PC2 push2;

RWStructuredBuffer<float> outBuf;

export float helper() { return push.a.x; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main() { outBuf[0] = 1; }

// CHECK-NOT: error[
// CHECK: layout(push_constant)
// CHECK-NEXT: uniform block_PC_0
// CHECK-NOT: layout(push_constant)
// CHECK: float helper_0()

// VIA-NOT: glslang
// VIA: PushConstant
