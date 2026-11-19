// The implicit offset qualifier the parser synthesizes for an `atomic_uint` with no
// explicit `offset` flows through the same check-time path: with a symbolic link-time
// binding it is diagnosed with the concrete-binding error too — silently skipping would
// corrupt the implicit-offset bookkeeping.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/bindings/link-time-vk-binding-glsl-implicit-offset.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly

#version 450

extern static const int SLOT = 1;

layout(binding = SLOT) uniform atomic_uint counter;
/*CHECK:
       ^^^^^^^ GLSL offset layout qualifier requires a concrete binding index
       ^^^^^^^ the GLSL 'offset' layout qualifier requires a concrete 'binding' index at compile time to track per-binding offsets; link-time constant expressions are not supported on this check-time path
*/

void computeMain()
{
}
