// Ray-AABB and ray-triangle intersection primitives. Includes a
// linear-scan fallback used only when traversal stack overflows.

import bvh_types;

public bool rayAabbIntersect(Ray ray, float3 bmin, float3 bmax)
{
    // Slab test. Branchless version is faster on GPU but for the
    // coverage demo we want each axis to be a distinct branch.
    float3 invDir = float3(1.0) / ray.dir;
    float3 t0 = (bmin - ray.origin) * invDir;
    float3 t1 = (bmax - ray.origin) * invDir;
    float3 tmin = min(t0, t1);
    float3 tmax = max(t0, t1);

    float tEnter = max(max(tmin.x, tmin.y), tmin.z);
    float tExit = min(min(tmax.x, tmax.y), tmax.z);

    if (tEnter > tExit)
        return false;
    if (tExit < ray.tMin)
        return false;
    if (tEnter > ray.tMax)
        return false;
    return true;
}

public bool rayTriangleIntersect(Ray ray, Triangle tri, out float t)
{
    // Möller-Trumbore
    float3 e1 = tri.v1 - tri.v0;
    float3 e2 = tri.v2 - tri.v0;
    float3 p = cross(ray.dir, e2);
    float det = dot(e1, p);

    if (det > -1e-8 && det < 1e-8)
    {
        // Ray parallel to triangle plane — rare branch, only hit by
        // grazing rays.
        t = 0.0;
        return false;
    }

    float invDet = 1.0 / det;
    float3 tvec = ray.origin - tri.v0;
    float u = dot(tvec, p) * invDet;
    if (u < 0.0 || u > 1.0)
    {
        t = 0.0;
        return false;
    }

    float3 q = cross(tvec, e1);
    float v = dot(ray.dir, q) * invDet;
    if (v < 0.0 || u + v > 1.0)
    {
        t = 0.0;
        return false;
    }

    float tHit = dot(e2, q) * invDet;
    if (tHit < ray.tMin || tHit > ray.tMax)
    {
        t = 0.0;
        return false;
    }
    t = tHit;
    return true;
}

// Linear-scan fallback used when traversal stack overflows. Tests
// every triangle in the scene — slow but bounded. Returns the count
// of hits considered (for coverage telemetry; the demo doesn't use
// the return value semantically).
public int linearScanRemaining(Ray ray, inout HitInfo hit, uint triCount)
{
    int hitsConsidered = 0;
    for (uint i = 0; i < triCount; ++i)
    {
        Triangle tri = triangles[i];
        if (isDegenerate(tri))
            continue;
        float t;
        if (rayTriangleIntersect(ray, tri, t) && t < hit.t)
        {
            hit.t = t;
            hit.triIndex = int(i);
            hit.materialId = tri.materialId;
        }
        ++hitsConsidered;
    }
    return hitsConsidered;
}
