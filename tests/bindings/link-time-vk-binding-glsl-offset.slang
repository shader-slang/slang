// The GLSL `offset` layout qualifier tracks per-binding offsets, which needs a concrete
// `binding` index at compile time; a symbolic link-time binding cannot participate, and
// linking a module later cannot help because checking never re-runs.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/bindings/link-time-vk-binding-glsl-offset.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly

#version 450

extern static const int SLOT = 1;

layout(binding = SLOT, offset = 4) uniform atomic_uint counter;
/*CHECK:
       ^^^^^^^ GLSL offset layout qualifier requires a concrete binding index
       ^^^^^^^ the GLSL 'offset' layout qualifier requires a concrete 'binding' index at compile time to track per-binding offsets; link-time constant expressions are not supported on this legacy GLSL path
*/

void computeMain()
{
}
