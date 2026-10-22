//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/parser/ternary-middle-operand-assignment-glsl.slang -target spirv -stage compute -entry main

// GLSL input shares the expression parser, so the GLSL form from shader-slang/slang#13870, with an
// assignment in the middle operand of `?:`, compiles too.

#version 450
layout(local_size_x = 1) in;
layout(std430, binding = 0) buffer Buf
{
    float v[];
} buf;

// The division stays behind the branch on `b > g`.
// CHECK: OpEntryPoint GLCompute %main "main"
// CHECK: OpFOrdGreaterThan
// CHECK: OpBranchConditional
// CHECK: OpLabel
// CHECK-NEXT: {{OpFDiv|OpFMul}}
void main()
{
    float b = buf.v[0];
    float g = buf.v[1];
    b > g ? b /= 2.0 : 0.0;
    buf.v[2] = b;
}
