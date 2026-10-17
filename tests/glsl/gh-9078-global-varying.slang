//TEST:SIMPLE_EX(filecheck=METAL): -lang glsl tests/glsl/gh-9078-global-varying.slang -stage fragment -entry main -target metal
//TEST:SIMPLE_EX(filecheck=NO_FRAGMENT): -lang glsl tests/glsl/gh-9078-global-varying.slang -stage fragment -entry main -target cuda
//TEST:SIMPLE_EX(filecheck=NO_FRAGMENT): -lang glsl tests/glsl/gh-9078-global-varying.slang -stage fragment -entry main -target cpp
//TEST:SIMPLE_EX(filecheck=WGSL): -lang glsl tests/glsl/gh-9078-global-varying.slang -stage fragment -entry main -target wgsl

// A GLSL-style global `out` variable used alongside a global `uniform` must compile for Metal
// and WGSL. CUDA and C++ do not support user-defined fragment varyings, so they must report that
// rather than fail an internal assertion.
// See https://github.com/shader-slang/slang/issues/9078.

// METAL: float4 f{{.*}} {{\[\[}}color(0){{\]\]}}
// METAL: struct GlobalParams
// METAL: float4 v
// METAL: {{\[\[}}fragment{{\]\]}}
// METAL: = { globalParams_{{[0-9]+}}->v_0 }

// WGSL: var<uniform> globalParams
// WGSL: @location(0) f{{.*}} : vec4<f32>
// WGSL: @fragment
// WGSL: ( globalParams_{{[0-9]+}}.v_0 )

// NO_FRAGMENT-NOT: assert failure
// NO_FRAGMENT: this target doesn't support this user-defined varying parameter
// NO_FRAGMENT-NOT: assert failure

#version 460
uniform vec4 v;
out vec4 f;
void main()
{
    f = v;
}
