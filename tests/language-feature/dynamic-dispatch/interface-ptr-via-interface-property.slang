// Calling a method on a pointer-to-interface obtained through a property
// defined in another interface should generate correct dispatch code.
// This is a regression test for GitHub issue #9816.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -conformance Perspective:Camera

interface Camera
{
    RayDesc sample_ray(float2 uv);
};

struct Perspective : Camera
{
    RayDesc sample_ray(float2 uv)
    {
        RayDesc ray;
        return ray;
    }
};

interface IScene
{
    property Camera *camera { get; }
};

struct Scene : IScene
{
    Camera *camera;
};

uniform Scene _scene;

// CHECK: OpEntryPoint
[shader("compute"), numthreads(1, 1, 1)]
void computeMain()
{
    RayDesc rayDesc = _scene.camera->sample_ray(float2(2));
}
