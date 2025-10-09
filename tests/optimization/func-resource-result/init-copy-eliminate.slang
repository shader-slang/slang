//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

// Test on a regression that caused a field to be initialized twice in the ctor.

// CHECK-COUNT-1: callOnce{{.*}}(int(1000))
// CHECK-COUNT-1: {{.*}}->origin_0 = 
int callOnce(int x)
{ 
    *result += 1.0f;  // cause some side effect so the call is not eliminated
     return 999;
}

public struct Ray
{
    public int calledOnce = callOnce(1000);
    public float3 origin = {};
    public float t_min = 0;
    public float3 dir = {};
    public float t_max = 0;

    public __init(float3 origin, float3 dir, float t_min = 0.f, float t_max = 1000.0)
    {
        this.origin = origin;
        this.dir = dir;
        this.t_min = t_min;
        this.t_max = t_max;
    }

    public RayDesc to_ray_desc() { return { origin, t_min, dir, t_max }; }
};

uniform float* result;

[numthreads(1,1,1)]
void computeMain()
{
    Ray r = Ray(float3(1,2,3), float3(4,5,6));
    RayDesc rd = r.to_ray_desc();
    *result = rd.Direction.x;
}
