// `readonly` on an image parameter is the GLSL memory qualifier, which marks the image
// non-writable.

//TEST:SIMPLE(filecheck=CHECK): -lang glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly

#version 450
layout(local_size_x = 1) in;

layout(binding = 0, rgba8) uniform readonly image2D srcImage;
layout(binding = 1, rgba8) uniform writeonly image2D dstImage;

// CHECK: OpDecorate %{{.*}} NonWritable
vec4 load(readonly image2D img)
{
    return imageLoad(img, ivec2(0, 0));
}

void computeMain()
{
    imageStore(dstImage, ivec2(0, 0), load(srcImage));
}
