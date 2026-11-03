//TEST:SIMPLE(filecheck=CHECK): -lang glsl -stage compute -entry main -target wgsl
//TEST:SIMPLE(filecheck=VERT): -lang glsl -stage vertex -entry vertMain -target wgsl
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent): a
// vertex shader must not use read_write storage.
//TEST:SIMPLE(filecheck=SPV): -lang glsl -stage vertex -entry vertMain -target wgsl-spirv-asm

// A GLSL `readonly buffer` is a `read` storage buffer in WGSL; other buffers stay read_write.

#version 450
layout(local_size_x = 1) in;

// CHECK-DAG: var<storage, read> In_{{[0-9]+}} : array<f32>;
// CHECK-DAG: var<storage, read> Params_{{[0-9]+}} : array<
// CHECK-DAG: var<storage, read_write> Out_{{[0-9]+}} : array<f32>;
layout(binding = 0) readonly buffer In { float vin[]; };
layout(binding = 1) buffer Out { float vout[]; };
layout(binding = 2) readonly buffer Params { uint count; float scale; };

void main()
{
    uint i = gl_GlobalInvocationID.x;
    if (i < count)
        vout[i] = vin[i] * scale;
}

// VERT: var<storage, read> Instances_{{[0-9]+}} : array<vec4<f32>>;
// SPV: OpEntryPoint
layout(location = 0) in vec4 inPos;
layout(location = 1) in uint inInstance;
layout(binding = 3) readonly buffer Instances { vec4 offsets[]; };

void vertMain()
{
    gl_Position = inPos + offsets[inInstance];
}
