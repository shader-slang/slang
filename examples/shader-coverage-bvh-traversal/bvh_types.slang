// Shared data types and global resources for the BVH traversal demo.
// Layout must match the host-side structs in main.cpp.

public struct Ray
{
    public float3 origin;
    public float tMin;
    public float3 dir;
    public float tMax;
}

public struct Triangle
{
    public float3 v0;
    public int materialId;
    public float3 v1;
    public int pad1;
    public float3 v2;
    public int pad2;
}

public struct BVHNode
{
    public float3 bmin;
    public int leftIndex;     // -1 if leaf
    public float3 bmax;
    public int rightIndex;
    public int firstTri;      // index into triangles[] (leaf only)
    public int triCount;      // primitives in this leaf
    public int pad0;
    public int pad1;
}

public struct Globals
{
    public uint rayCount;
    public uint triCount;
    public uint nodeCount;
    public uint pad;
}

public struct HitInfo
{
    public float t;
    public int triIndex;
    public int materialId;
}

[[vk::binding(0, 0)]] public StructuredBuffer<Ray> rays;
[[vk::binding(1, 0)]] public StructuredBuffer<Triangle> triangles;
[[vk::binding(2, 0)]] public StructuredBuffer<BVHNode> nodes;
[[vk::binding(3, 0)]] public StructuredBuffer<Globals> globals;
[[vk::binding(4, 0)]] public RWStructuredBuffer<float4> output;

public bool isDegenerate(Triangle tri)
{
    float3 e1 = tri.v1 - tri.v0;
    float3 e2 = tri.v2 - tri.v0;
    float3 n = cross(e1, e2);
    return dot(n, n) < 1e-12;
}

public float3 triangleNormal(Triangle tri)
{
    float3 e1 = tri.v1 - tri.v0;
    float3 e2 = tri.v2 - tri.v0;
    return normalize(cross(e1, e2));
}
