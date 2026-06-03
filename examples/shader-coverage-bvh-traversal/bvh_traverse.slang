// Software BVH traversal — iterative stack-based ray traversal through
// a BVH built on the CPU. Multiple material kinds; rare paths
// (stack-overflow fallback, debug material, degenerate-triangle
// branch) only trigger for specific inputs. Coverage delta tells the
// story.

import bvh_types;
import ray_intersect;
import materials;

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Globals g = globals[0];
    uint rayIndex = tid.x;
    if (rayIndex >= g.rayCount)
        return;

    Ray ray = rays[rayIndex];

    HitInfo hit;
    hit.t = ray.tMax;
    hit.triIndex = -1;
    hit.materialId = -1;

    // Iterative traversal with a small per-thread stack.
    int stack[24];
    int stackPtr = 0;
    stack[stackPtr++] = 0; // root

    int overflowFallbackHits = 0;

    while (stackPtr > 0)
    {
        if (stackPtr >= 24)
        {
            // Stack-overflow fallback: linear scan over remaining
            // primitives instead of recursing further. Triggers only
            // for pathological rays through deeply imbalanced BVHs.
            overflowFallbackHits = linearScanRemaining(ray, hit, g.triCount);
            break;
        }

        int nodeId = stack[--stackPtr];
        BVHNode node = nodes[nodeId];

        if (!rayAabbIntersect(ray, node.bmin, node.bmax))
            continue;

        if (node.leftIndex < 0)
        {
            // Leaf node — iterate triangles in this leaf
            int start = node.firstTri;
            int count = node.triCount;
            for (int i = 0; i < count; ++i)
            {
                int triIdx = start + i;
                Triangle tri = triangles[triIdx];
                if (isDegenerate(tri))
                {
                    // Degenerate triangle (zero area) — skip without
                    // running the full intersect path. Only present
                    // in the stress mesh.
                    continue;
                }
                float t;
                if (rayTriangleIntersect(ray, tri, t) && t < hit.t)
                {
                    hit.t = t;
                    hit.triIndex = triIdx;
                    hit.materialId = tri.materialId;
                }
            }
        }
        else
        {
            // Internal node — push both children. Order matters for
            // performance (push the farther child first so the closer
            // child gets popped first), but for the demo we use a
            // simple direction-dependent ordering branch.
            int leftChild = node.leftIndex;
            int rightChild = node.rightIndex;
            if (ray.dir.x >= 0.0)
            {
                stack[stackPtr++] = rightChild;
                stack[stackPtr++] = leftChild;
            }
            else
            {
                stack[stackPtr++] = leftChild;
                stack[stackPtr++] = rightChild;
            }
        }
    }

    // Material dispatch — only some materials present in the smoke
    // scene; the stress scene exercises all four.
    float3 shade = float3(0.0, 0.0, 0.0);
    if (hit.triIndex >= 0)
    {
        Triangle tri = triangles[hit.triIndex];
        float3 surfaceNormal = triangleNormal(tri);
        shade = evaluateMaterial(hit.materialId, ray.dir, surfaceNormal, hit.t);
    }
    else
    {
        // Miss — sky background gradient
        shade = skyGradient(ray.dir);
    }

    output[rayIndex] = float4(shade, hit.t);
}
