// When both a GLSL layout(local_size_*) qualifier and a Slang [numthreads]
// qualifier are specified, the [numthreads] attribute should be preferred
// in reflection info and in output. See https://github.com/shader-slang/slang/issues/7894.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main

#version 460
layout(local_size_x = 4, local_size_y = 5, local_size_z = 6) in;

[numthreads(2, 4, 8)]
void main() {}

// CHECK: OpExecutionMode %main LocalSize 2 4 8