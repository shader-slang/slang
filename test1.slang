public interface Camera
{
    public RayDesc sample_ray();
};

public struct Perspective : Camera
{
    public RayDesc sample_ray()
    {
        RayDesc ray = {};
        return ray;
    }
};

public interface IScene
{
    public property Camera *camera;
};

struct Scene : IScene {
    Camera *camera;
};

uniform Scene _scene;

[shader("compute"), numthreads(16, 16, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    RayDesc rayDesc = _scene.camera->sample_ray();
}
