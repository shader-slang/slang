// A GLSL `writeonly` image keeps the GLSL rules: writing through the subscript or the image's
// methods stays legal, and a method receiver is not restricted the way a `__ref_writeonly`
// parameter's receiver is. (Reads of GLSL `writeonly` storage through members and elements are
// reported by the shared coercion check; reads through methods are not yet, pending the GLSL
// unification the issue defers.)

//TEST:SIMPLE: -target spirv -entry main -stage compute -no-codegen

uniform writeonly RWTexture2D<float> g_image;

[numthreads(1, 1, 1)]
void main()
{
    uint width, height;
    g_image[uint2(0, 0)] = 1.0;
    g_image.Store(uint2(0, 0), 2.0);
    g_image.GetDimensions(width, height);
    float value = g_image.Load(uint2(0, 0));
}
