// When both a GLSL layout(local_size_*) qualifier and a Slang [numthreads]
// qualifier are specified, the [numthreads] attribute should be preferred
// in reflection info and in output. See https://github.com/shader-slang/slang/issues/7894.

//TEST:SIMPLE(filecheck=CHECK):-target spirv

#version 460
layout(constant_id = 2) const int specConst2 = 999;
layout(constant_id = 3) const int specConst3 = 999;
layout(constant_id = 4) const int specConst4 = 999;
layout(constant_id = 5) const int specConst5 = 999;
layout(constant_id = 6) const int specConst6 = 999;

layout(local_size_x_id = 2, local_size_y = 3, local_size_z_id = 4) in;

// All 3 of these should override the layout() directive
// CHECK: OpExecutionModeId %main LocalSizeId %int_7 %specConst5 %specConst6
[numthreads(7, specConst5, specConst6)]
[shader("compute")]
void main() {}

// And the thread size here should come from the layout() directive
// CHECK: OpExecutionModeId %main2 LocalSizeId %specConst2 %int_3 %specConst4
[shader("compute")]
void main2() {}
