#version 450
// We spell CUDA surface accesses only for 1D, 2D and 3D textures and 1D and 2D array textures,
// so a write through the subscript of a cube image is rejected on CUDA (shader-slang/slang#12627).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -lang glsl -target cuda -entry main -stage compute

layout(local_size_x = 1) in;
layout(r32f) uniform imageCube gImage;

void main()
{
    gImage[ivec3(0)].x = 1.0f;
//CHECK:               ^ error E56008
//CHECK:               ^ this write to a texel is lowered to CUDA surface calls, which Slang spells only for 1D, 2D and 3D textures and 1D and 2D array textures.
}
