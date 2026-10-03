#include "slang-cpp-prelude.h"

#include <cmath>
#include <new>
#include <stdio.h>

struct Triangle
{
    float3 a, b, c;
    float3 albedo;
    float3 emission;
};

static float dot(const float3& a, const float3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static float3 cross(const float3& a, const float3& b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// Geometry is immutable during dispatch. Every query scans this list independently, with no BVH.
struct TriangleScene : IRaytracingAccelerationStructure
{
    Triangle triangles[36] = {};
    uint32_t triangleCount = 0;

    bool proceed(RayQueryState* state) const override
    {
        const uint32_t flags = state->rayFlags;
        const bool opaque = !(flags & SLANG_RAY_QUERY_FLAG_FORCE_NON_OPAQUE);
        if (!(state->instanceInclusionMask & 0xff) ||
            (flags & SLANG_RAY_QUERY_FLAG_SKIP_TRIANGLES) ||
            (opaque && (flags & SLANG_RAY_QUERY_FLAG_CULL_OPAQUE)) ||
            (!opaque && (flags & SLANG_RAY_QUERY_FLAG_CULL_NON_OPAQUE)))
        {
            state->traversalPhase = SLANG_RAY_QUERY_TRAVERSAL_COMPLETE;
            return false;
        }

        const float3 origin = _slangRayQueryGetFloat3(state->worldRayOrigin);
        const float3 direction = _slangRayQueryGetFloat3(state->worldRayDirection);
        // Use the query's leaf cursor as the next triangle index, including across candidate
        // yields.
        for (uint32_t i = state->tlasLeafOffset; i < triangleCount; ++i)
        {
            state->tlasLeafOffset = i + 1;
            const Triangle& triangle = triangles[i];
            const float3 edge1 = triangle.b - triangle.a;
            const float3 edge2 = triangle.c - triangle.a;
            const float3 p = cross(direction, edge2);
            const float determinant = dot(edge1, p);
            const bool frontFace = determinant > 0;
            if (determinant == 0 ||
                (frontFace && (flags & SLANG_RAY_QUERY_FLAG_CULL_FRONT_FACING_TRIANGLES)) ||
                (!frontFace && (flags & SLANG_RAY_QUERY_FLAG_CULL_BACK_FACING_TRIANGLES)))
            {
                continue;
            }
            const float3 offset = origin - triangle.a;
            const float3 q = cross(offset, edge1);
            const float u = dot(offset, p) / determinant;
            const float v = dot(direction, q) / determinant;
            const float rayT = dot(edge2, q) / determinant;
            if (!(u >= 0 && v >= 0 && u + v <= 1 && rayT >= state->rayTMin &&
                  rayT <= state->committed.rayT))
            {
                continue;
            }

            RayQueryHit hit = {};
            hit.rayT = rayT;
            hit.barycentrics[0] = u;
            hit.barycentrics[1] = v;
            hit.primitiveIndex = i;
            hit.triangleFrontFace = frontFace;
            for (uint32_t axis = 0; axis < 3; ++axis)
            {
                hit.objectRayOrigin[axis] = origin[axis];
                hit.objectRayDirection[axis] = direction[axis];
                hit.objectToWorld[axis * 4 + axis] = hit.worldToObject[axis * 4 + axis] = 1;
            }
            if (opaque)
            {
                state->committed = hit;
                state->committedStatus = SLANG_RAY_QUERY_COMMITTED_TRIANGLE_HIT;
                if (!(flags & SLANG_RAY_QUERY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH))
                    continue;
            }
            else
            {
                state->candidate = hit;
                state->candidateType = SLANG_RAY_QUERY_CANDIDATE_NON_OPAQUE_TRIANGLE;
                state->candidatePending = 1;
                if (i + 1 == triangleCount)
                    state->traversalPhase = SLANG_RAY_QUERY_TRAVERSAL_COMPLETE;
                return true;
            }
            break;
        }
        state->traversalPhase = SLANG_RAY_QUERY_TRAVERSAL_COMPLETE;
        return false;
    }

    void addQuad(float3 a, float3 b, float3 c, float3 d, float3 albedo, float3 emission = {0, 0, 0})
    {
        assert(triangleCount + 2 <= 36);
        triangles[triangleCount++] = {a, b, c, albedo, emission};
        triangles[triangleCount++] = {a, c, d, albedo, emission};
    }

    void addBox(float x, float z, float halfWidth, float height, float angle)
    {
        float3 corners[8];
        for (uint32_t i = 0; i < 8; ++i)
        {
            const float localX = (i & 1) ? halfWidth : -halfWidth;
            const float localZ = (i & 2) ? halfWidth : -halfWidth;
            corners[i] = {
                x + localX * std::cos(angle) + localZ * std::sin(angle),
                (i & 4) ? height : 0,
                z - localX * std::sin(angle) + localZ * std::cos(angle)};
        }
        const float3 white = {0.73f, 0.73f, 0.73f};
        addQuad(corners[0], corners[1], corners[3], corners[2], white);
        addQuad(corners[4], corners[6], corners[7], corners[5], white);
        addQuad(corners[0], corners[4], corners[5], corners[1], white);
        addQuad(corners[2], corners[3], corners[7], corners[6], white);
        addQuad(corners[0], corners[2], corners[6], corners[4], white);
        addQuad(corners[1], corners[5], corners[7], corners[3], white);
    }

    void makeCornellBox()
    {
        const float3 white = {0.73f, 0.73f, 0.73f};
        addQuad({-1, 0, 0}, {1, 0, 0}, {1, 0, 2}, {-1, 0, 2}, white);
        addQuad({-1, 2, 0}, {-1, 2, 2}, {1, 2, 2}, {1, 2, 0}, white);
        addQuad({-1, 0, 2}, {1, 0, 2}, {1, 2, 2}, {-1, 2, 2}, white);
        addQuad({-1, 0, 0}, {-1, 0, 2}, {-1, 2, 2}, {-1, 2, 0}, {0.65f, 0.06f, 0.04f});
        addQuad({1, 0, 0}, {1, 2, 0}, {1, 2, 2}, {1, 0, 2}, {0.07f, 0.45f, 0.10f});
        addBox(-0.43f, 0.65f, 0.32f, 0.62f, -0.28f);
        addBox(0.40f, 1.28f, 0.32f, 1.18f, 0.30f);
        // Keep the light last so the shader can sample the same geometry used for intersections.
        addQuad(
            {-0.35f, 1.99f, 0.65f},
            {0.35f, 1.99f, 0.65f},
            {0.35f, 1.99f, 1.25f},
            {-0.35f, 1.99f, 1.25f},
            {0, 0, 0},
            {15, 13, 10});
    }
};

// Check traversal independently of shading, including a nearer triangle later in the list.
static bool testQueries()
{
    TriangleScene scene;
    scene.addQuad({0, 0, 2}, {1, 0, 2}, {1, 1, 2}, {0, 1, 2}, {1, 1, 1});
    scene.addQuad({0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}, {1, 1, 1});
    RayQuery<0> query;
    for (uint32_t test = 0; test < 10; ++test)
    {
        uint32_t flags =
            test == 0 ? SLANG_RAY_QUERY_FLAG_FORCE_OPAQUE : SLANG_RAY_QUERY_FLAG_FORCE_NON_OPAQUE;
        if (test == 7)
            flags |= SLANG_RAY_QUERY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
        if (test == 8)
            flags |= SLANG_RAY_QUERY_FLAG_SKIP_TRIANGLES;
        if (test == 9)
            flags |= SLANG_RAY_QUERY_FLAG_CULL_BACK_FACING_TRIANGLES;
        const RayDesc ray = {
            test == 4 ? float3{2, 2, 0} : float3{0.25f, 0.2f, 0},
            0,
            {0, 0, 1},
            test == 6 ? 0.5f : 10};
        query.TraceRayInline({&scene}, flags, test == 5 ? 0 : 0xff, ray);
        uint32_t candidates = 0;
        while (query.Proceed())
        {
            ++candidates;
            if (test == 3)
                query.Abort();
            else if (test != 2)
                query.CommitNonOpaqueTriangleHit();
        }
        const bool hit = test == 0 || test == 1 || test == 7;
        const uint32_t expectedCandidates = test == 1 || test == 2   ? 2
                                            : test == 3 || test == 7 ? 1
                                                                     : 0;
        if (query.CommittedStatus() != (hit ? SLANG_RAY_QUERY_COMMITTED_TRIANGLE_HIT : 0) ||
            candidates != expectedCandidates || query.state.candidatePending ||
            (hit && (query.CommittedRayT() != (test == 7 ? 2 : 1) ||
                     query.CommittedPrimitiveIndex() != (test == 7 ? 0 : 2))))
        {
            fprintf(stderr, "CPU RayQuery case %u failed.\n", test);
            return false;
        }
    }
    scene.triangles[0] = scene.triangles[2];
    scene.triangleCount = 1;
    const RayDesc ray = {{0.25f, 0.2f, 0}, 0, {0, 0, 1}, 10};
    query.TraceRayInline({&scene}, SLANG_RAY_QUERY_FLAG_FORCE_NON_OPAQUE, 0xff, ray);
    if (!(query.Proceed() && query.state.candidatePending && !query.Proceed() &&
          !query.state.candidatePending))
    {
        fprintf(stderr, "CPU RayQuery final candidate did not expire.\n");
        return false;
    }
    return true;
}

// Field order and element layouts match the shader globals, using only CPU prelude types.
struct GlobalParams
{
    RaytracingAccelerationStructure scene;
    StructuredBuffer<Triangle> triangles;
    RWStructuredBuffer<float3> pixels;
    uint32_t imageSize;
    uint32_t samplesPerPixel;
    uint32_t lightTriangleIndex;
};

extern "C" void computeMain(ComputeVaryingInput*, void*, void*);

int main(int argc, char** argv)
{
    const bool test = argc == 2 && strcmp(argv[1], "--test") == 0;
    const char* outputPath = argc > 1 ? argv[1] : "cornell-box.ppm";
    const int imageSize = test ? 32 : argc > 2 ? atoi(argv[2]) : 256;
    const int samples = test ? 4 : argc > 3 ? atoi(argv[3]) : 64;
    if (argc > 4 || imageSize < 1 || imageSize > 2048 || samples < 1 || samples > 4096)
    {
        fprintf(stderr, "Usage: cpu-ray-query [output.ppm] [size: 1..2048] [samples: 1..4096]\n");
        return 1;
    }
    if (test && !testQueries())
        return 1;

    TriangleScene scene;
    scene.makeCornellBox();
    const size_t pixelCount = size_t(imageSize) * imageSize;
    float3* pixels = new (std::nothrow) float3[pixelCount];
    if (!pixels)
    {
        fprintf(stderr, "Could not allocate image pixels.\n");
        return 1;
    }
    GlobalParams globals = {
        {&scene},
        {scene.triangles, scene.triangleCount},
        {pixels, pixelCount},
        uint32_t(imageSize),
        uint32_t(samples),
        scene.triangleCount - 2};
    ComputeVaryingInput groups = {{0, 0, 0}, {uint32_t(imageSize), uint32_t(imageSize), 1}};
    computeMain(&groups, nullptr, &globals);

    bool valid = true, hasRed = false, hasGreen = false, hasLight = false;
    for (size_t i = 0; i < pixelCount; ++i)
    {
        for (uint32_t channel = 0; channel < 3; ++channel)
            valid &= std::isfinite(pixels[i][channel]) && pixels[i][channel] >= 0;
        hasRed |= pixels[i].x > pixels[i].y * 2 + 0.02f;
        hasGreen |= pixels[i].y > pixels[i].x * 2 + 0.02f;
        hasLight |= pixels[i].x > 1;
    }
    if (test)
        valid &= hasRed && hasGreen && hasLight;
    if (!valid)
        fprintf(stderr, "Cornell box render failed: missing colors/light or invalid radiance.\n");

    if (valid && !test)
    {
        FILE* file = fopen(outputPath, "wb");
        if (!file)
            valid = false;
        else
        {
            valid = fprintf(file, "P6\n%d %d\n255\n", imageSize, imageSize) > 0;
            for (size_t i = 0; valid && i < pixelCount; ++i)
            {
                unsigned char rgb[3];
                for (uint32_t channel = 0; channel < 3; ++channel)
                {
                    const float exposed = pixels[i][channel] * 1.8f;
                    const float mapped = std::pow(exposed / (1 + exposed), 1.0f / 2.2f);
                    rgb[channel] = (unsigned char)(mapped * 255 + 0.5f);
                }
                valid = fwrite(rgb, 1, 3, file) == 3;
            }
            valid &= fclose(file) == 0;
        }
        if (valid)
            printf(
                "Wrote %s (%d x %d, %d samples/pixel).\n",
                outputPath,
                imageSize,
                imageSize,
                samples);
        else
            fprintf(stderr, "Could not write %s.\n", outputPath);
    }
    delete[] pixels;
    if (test && valid)
        puts("Standalone Cornell box and RayQuery tests passed.");
    return valid ? 0 : 1;
}
