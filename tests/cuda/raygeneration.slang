//TEST:SIMPLE(filecheck=CHECK): -target cuda
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx

// Test that we emit a valid raygeneration kernel.
// This test will fail if either: (1) `OptixTraversableHandle` is not emitted
// as a local var for `RaytracingAccelerationStructure` or (2)`RaytracingAccelerationStructure`
// is not hoisted into use-site.

// CHECK: void{{.*}}raygenMain
// CHECK-PTX: .visible .entry{{.*}}raygenMain

struct Ray {
    float3 origin;
    float t_min;
    float3 dir;
    float t_max;

    __init(float3 origin, float3 dir, float t_min = 0.f, float t_max = 1000.0)
    {
        this.origin = origin;
        this.dir = dir;
        this.t_min = t_min;
        this.t_max = t_max;
    }

    RayDesc to_ray_desc() { return { origin, t_min, dir, t_max }; }
};


struct Camera {
    float3 position;
    float3 image_u;
    float3 image_v;
    float3 image_w;

    Ray get_ray(float2 uv)
    {
        uv = uv * 2 - 1;
        float3 dir = normalize(uv.x * image_u + uv.y * image_v + image_w);
        return Ray(position, dir);
    }
};

struct Scene {
    RaytracingAccelerationStructure tlas;

    Camera camera;
};

struct Path {
    uint2 pixel;
    uint vertex_index;
    Ray ray;
    float3 thp;
    float3 L;
    int rng;

    __init(uint2 pixel, Ray ray, int rng)
    {
        this.pixel = pixel;
        this.vertex_index = 0;
        this.ray = ray;
        this.thp = float3(1);
        this.L = float3(0);
        this.rng = rng;
    }
};
ParameterBlock<Scene> g_scene;
RWTexture2D<float4> g_output;

[require(sm_6_8, cuda)]
[shader("raygeneration")]
void raygenMain()
{
    uint2 pixel = DispatchRaysIndex().xy;
    float3 L = float3(0);
    Ray ray = g_scene.camera.get_ray(pixel.xy);
    Path path = Path(pixel, ray, 1);
    TraceRay(
        g_scene.tlas,
        0,
        0xff,
        0,
        0,
        0,
        path.ray.to_ray_desc(),
        path
    );
    L += path.L;
    g_output[pixel] = float4(L, 1);
}
