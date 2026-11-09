//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain -force-glsl-scalar-layout
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain -fvk-use-c-layout

// Entry-point uniforms are placed in a `var<uniform>`, where WGSL requires a 16-byte array
// stride. Forcing the scalar or C layout must not drop the std140 array wrapper there, or the
// shader is rejected at creation and no longer matches the reflected offsets.

// CHECK: struct _Array_std140_int2_0
// CHECK: data_0 : array<vec4<i32>, i32(2)>
// CHECK: struct EntryPointParams_std140_0
// CHECK: stride_0 : _Array_std140_int2_0
// CHECK: var<uniform> entryPointParams_0 : EntryPointParams_std140_0;
// CHECK-NOT: array<i32, i32(2)>

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uniform int stride[2], uniform float scale)
{
    outputBuffer[0] = stride[0] + stride[1] + int(scale);
}
