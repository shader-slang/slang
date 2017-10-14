//TEST(smoke):REFLECTION:-profile ps_4_0 -target glsl

// Confirm that we expose GLSL `image` types through reflection

layout(rgba32f)
uniform writeonly imageBuffer iBuffer;

layout(rgba32f)
uniform writeonly image2D i2D;

void main()
{}
