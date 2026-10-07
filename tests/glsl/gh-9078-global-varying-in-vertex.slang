//TEST:SIMPLE_EX(filecheck=METAL): -lang glsl tests/glsl/gh-9078-global-varying-in-vertex.slang -stage vertex -entry main -target metal
//TEST:SIMPLE_EX(filecheck=WGSL): -lang glsl tests/glsl/gh-9078-global-varying-in-vertex.slang -stage vertex -entry main -target wgsl
//TEST:SIMPLE_EX(filecheck=NO_VARYING): -lang glsl tests/glsl/gh-9078-global-varying-in-vertex.slang -stage vertex -entry main -target cuda
//TEST:SIMPLE_EX(filecheck=NO_VARYING): -lang glsl tests/glsl/gh-9078-global-varying-in-vertex.slang -stage vertex -entry main -target cpp

// See https://github.com/shader-slang/slang/issues/9078.

// METAL: float4 c{{.*}} {{\[\[}}user(COLOR){{\]\]}}
// METAL: float4 p{{.*}} {{\[\[}}attribute(0){{\]\]}}
// METAL: struct GlobalParams
// METAL: {{\[\[}}vertex{{\]\]}}
// METAL: = globalParams_{{[0-9]+}}->v_0;

// WGSL: var<uniform> globalParams
// WGSL: @location(0) c{{.*}} : vec4<f32>
// WGSL: @location(0) p{{.*}} : vec4<f32>
// WGSL: @vertex
// WGSL: = globalParams_{{[0-9]+}}.v_0;

// NO_VARYING-NOT: assert failure
// NO_VARYING: this target doesn't support this user-defined varying parameter
// NO_VARYING-NOT: assert failure

#version 460
uniform vec4 v;
in vec4 p;
out vec4 c;
void main()
{
    gl_Position = p;
    c = v;
}
