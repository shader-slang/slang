#ifndef SLANG_PRELUDE_CPP_RAY_QUERY_H
#define SLANG_PRELUDE_CPP_RAY_QUERY_H

// This header is included from slang-cpp-types.h while SLANG_PRELUDE_NAMESPACE is open.
// It defines the CPU RayQuery ABI shared by generated C++ and the CPU RHI backend. The ABI is
// intentionally independent of the acceleration-structure implementation.

struct RayQueryState;

struct IRaytracingAccelerationStructure
{
    // Resumes traversal until a non-opaque triangle or procedural primitive candidate is found,
    // or traversal completes.
    // All mutable traversal data belongs to `state`; the acceleration structure remains read-only.
    virtual bool proceed(RayQueryState* state) const = 0;
};

struct RaytracingAccelerationStructure
{
    IRaytracingAccelerationStructure* handle;
};

struct RayDesc
{
    float3 Origin;
    float TMin;
    float3 Direction;
    float TMax;
};

enum : uint32_t
{
    SLANG_RAY_QUERY_FLAG_FORCE_OPAQUE = 0x01,
    SLANG_RAY_QUERY_FLAG_FORCE_NON_OPAQUE = 0x02,
    SLANG_RAY_QUERY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH = 0x04,
    SLANG_RAY_QUERY_FLAG_CULL_BACK_FACING_TRIANGLES = 0x10,
    SLANG_RAY_QUERY_FLAG_CULL_FRONT_FACING_TRIANGLES = 0x20,
    SLANG_RAY_QUERY_FLAG_CULL_OPAQUE = 0x40,
    SLANG_RAY_QUERY_FLAG_CULL_NON_OPAQUE = 0x80,
    SLANG_RAY_QUERY_FLAG_SKIP_TRIANGLES = 0x100,
    SLANG_RAY_QUERY_FLAG_SKIP_PROCEDURAL_PRIMITIVES = 0x200,
};

enum : uint32_t
{
    SLANG_RAY_QUERY_COMMITTED_NOTHING = 0,
    SLANG_RAY_QUERY_COMMITTED_TRIANGLE_HIT = 1,
    SLANG_RAY_QUERY_COMMITTED_PROCEDURAL_PRIMITIVE_HIT = 2,
};

enum : uint32_t
{
    SLANG_RAY_QUERY_CANDIDATE_NON_OPAQUE_TRIANGLE = 0,
    SLANG_RAY_QUERY_CANDIDATE_PROCEDURAL_PRIMITIVE = 1,
};

enum : uint32_t
{
    SLANG_RAY_QUERY_TRAVERSAL_COMPLETE = 0,
    SLANG_RAY_QUERY_TRAVERSAL_TLAS = 1,
    SLANG_RAY_QUERY_TRAVERSAL_BLAS = 2,
};

struct RayQueryHit
{
    float rayT;
    float barycentrics[2];
    float objectRayOrigin[3];
    float objectRayDirection[3];
    float objectToWorld[12];
    float worldToObject[12];
    uint32_t instanceIndex;
    uint32_t instanceID;
    uint32_t instanceContributionToHitGroupIndex;
    uint32_t geometryIndex;
    uint32_t primitiveIndex;
    uint32_t triangleFrontFace;
    uint32_t proceduralPrimitiveNonOpaque;
};

struct RayQueryState
{
    // TinyBVH uses these same maximum stack depths for its scalar BVH/TLAS traversal. Keeping the
    // cursor in the query object makes Proceed genuinely resumable without allocating or replaying.
    static const uint32_t kTLASStackCapacity = 64;
    static const uint32_t kBLASStackCapacity = 256;
    static const uint32_t kInvalidNode = 0xffffffffu;

    IRaytracingAccelerationStructure* accelerationStructure;

    float worldRayOrigin[3];
    float worldRayDirection[3];
    float rayTMin;
    float rayTMax;

    uint32_t rayFlags;
    uint32_t instanceInclusionMask;
    uint32_t traversalPhase;
    uint32_t candidatePending;
    uint32_t candidateType;
    uint32_t committedStatus;

    uint32_t tlasNode;
    uint32_t tlasLeafOffset;
    uint32_t tlasStackSize;
    uint32_t tlasStack[kTLASStackCapacity];

    uint32_t blasNode;
    uint32_t blasLeafOffset;
    uint32_t blasStackSize;
    uint32_t blasStack[kBLASStackCapacity];
    uint32_t currentInstanceIndex;

    RayQueryHit candidate;
    RayQueryHit committed;
};

SLANG_FORCE_INLINE float3 _slangRayQueryGetFloat3(const float value[3])
{
    return float3{value[0], value[1], value[2]};
}

SLANG_FORCE_INLINE float2 _slangRayQueryGetFloat2(const float value[2])
{
    return float2{value[0], value[1]};
}

template<int ROWS, int COLS>
SLANG_FORCE_INLINE Matrix<float, ROWS, COLS> _slangRayQueryGetMatrix(
    const float value[12],
    bool transpose)
{
    Matrix<float, ROWS, COLS> result;
    for (int row = 0; row < ROWS; ++row)
    {
        for (int column = 0; column < COLS; ++column)
        {
            result.rows[row][column] =
                transpose ? value[column * 4 + row] : value[row * 4 + column];
        }
    }
    return result;
}

template<uint32_t rayFlagsGeneric>
struct RayQuery
{
    RayQuery()
    {
        state = {};
        state.tlasNode = RayQueryState::kInvalidNode;
        state.blasNode = RayQueryState::kInvalidNode;
    }

    SLANG_FORCE_INLINE void TraceRayInline(
        RaytracingAccelerationStructure accelerationStructure,
        uint32_t rayFlags,
        uint32_t instanceInclusionMask,
        const RayDesc& ray)
    {
        state = {};
        state.accelerationStructure = accelerationStructure.handle;
        state.worldRayOrigin[0] = ray.Origin.x;
        state.worldRayOrigin[1] = ray.Origin.y;
        state.worldRayOrigin[2] = ray.Origin.z;
        state.worldRayDirection[0] = ray.Direction.x;
        state.worldRayDirection[1] = ray.Direction.y;
        state.worldRayDirection[2] = ray.Direction.z;
        state.rayTMin = ray.TMin;
        state.rayTMax = ray.TMax;
        state.rayFlags = rayFlags | rayFlagsGeneric;
        state.instanceInclusionMask = instanceInclusionMask;
        const bool skipAllGeometry =
            (state.rayFlags & SLANG_RAY_QUERY_FLAG_SKIP_TRIANGLES) &&
            (state.rayFlags & SLANG_RAY_QUERY_FLAG_SKIP_PROCEDURAL_PRIMITIVES);
        state.traversalPhase = state.accelerationStructure && !skipAllGeometry
                                   ? SLANG_RAY_QUERY_TRAVERSAL_TLAS
                                   : SLANG_RAY_QUERY_TRAVERSAL_COMPLETE;
        state.tlasNode = state.traversalPhase == SLANG_RAY_QUERY_TRAVERSAL_TLAS
                             ? 0
                             : RayQueryState::kInvalidNode;
        state.blasNode = RayQueryState::kInvalidNode;
        state.committed.rayT = ray.TMax;
        state.committedStatus = SLANG_RAY_QUERY_COMMITTED_NOTHING;
    }

    SLANG_FORCE_INLINE bool Proceed()
    {
        if (!state.accelerationStructure ||
            state.traversalPhase == SLANG_RAY_QUERY_TRAVERSAL_COMPLETE)
        {
            return false;
        }

        // A pending candidate that was not committed before the next Proceed is ignored.
        state.candidatePending = 0;
        return state.accelerationStructure->proceed(&state);
    }

    SLANG_FORCE_INLINE void Abort()
    {
        state.candidatePending = 0;
        state.traversalPhase = SLANG_RAY_QUERY_TRAVERSAL_COMPLETE;
        state.tlasNode = RayQueryState::kInvalidNode;
        state.blasNode = RayQueryState::kInvalidNode;
    }

    SLANG_FORCE_INLINE void CommitNonOpaqueTriangleHit()
    {
        if (!state.candidatePending ||
            state.candidateType != SLANG_RAY_QUERY_CANDIDATE_NON_OPAQUE_TRIANGLE)
        {
            return;
        }

        if (state.committedStatus == SLANG_RAY_QUERY_COMMITTED_NOTHING ||
            state.candidate.rayT < state.committed.rayT)
        {
            state.committed = state.candidate;
            state.committedStatus = SLANG_RAY_QUERY_COMMITTED_TRIANGLE_HIT;
        }
        state.candidatePending = 0;

        if (state.rayFlags & SLANG_RAY_QUERY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH)
        {
            state.traversalPhase = SLANG_RAY_QUERY_TRAVERSAL_COMPLETE;
            state.tlasNode = RayQueryState::kInvalidNode;
            state.blasNode = RayQueryState::kInvalidNode;
        }
    }

    SLANG_FORCE_INLINE void CommitProceduralPrimitiveHit(float rayT)
    {
        if (!state.candidatePending ||
            state.candidateType != SLANG_RAY_QUERY_CANDIDATE_PROCEDURAL_PRIMITIVE || rayT != rayT ||
            rayT < state.rayTMin || rayT > state.rayTMax)
        {
            return;
        }

        bool accepted = false;
        if (state.committedStatus == SLANG_RAY_QUERY_COMMITTED_NOTHING ||
            rayT < state.committed.rayT)
        {
            state.committed = state.candidate;
            state.committed.rayT = rayT;
            state.committedStatus = SLANG_RAY_QUERY_COMMITTED_PROCEDURAL_PRIMITIVE_HIT;
            accepted = true;
        }

        if (accepted && (state.rayFlags & SLANG_RAY_QUERY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH))
        {
            state.candidatePending = 0;
            state.traversalPhase = SLANG_RAY_QUERY_TRAVERSAL_COMPLETE;
            state.tlasNode = RayQueryState::kInvalidNode;
            state.blasNode = RayQueryState::kInvalidNode;
        }
    }

    SLANG_FORCE_INLINE uint32_t CandidateType() const { return state.candidateType; }
    SLANG_FORCE_INLINE uint32_t CommittedStatus() const { return state.committedStatus; }
    SLANG_FORCE_INLINE bool CandidateProceduralPrimitiveNonOpaque() const
    {
        return state.candidate.proceduralPrimitiveNonOpaque != 0;
    }

    SLANG_FORCE_INLINE float CandidateTriangleRayT() const { return state.candidate.rayT; }
    SLANG_FORCE_INLINE float CommittedRayT() const { return state.committed.rayT; }

    SLANG_FORCE_INLINE uint32_t CandidateInstanceContributionToHitGroupIndex() const
    {
        return state.candidate.instanceContributionToHitGroupIndex;
    }
    SLANG_FORCE_INLINE uint32_t CommittedInstanceContributionToHitGroupIndex() const
    {
        return state.committed.instanceContributionToHitGroupIndex;
    }

    SLANG_FORCE_INLINE uint32_t CandidateInstanceIndex() const
    {
        return state.candidate.instanceIndex;
    }
    SLANG_FORCE_INLINE uint32_t CommittedInstanceIndex() const
    {
        return state.committed.instanceIndex;
    }
    SLANG_FORCE_INLINE uint32_t CandidateInstanceID() const { return state.candidate.instanceID; }
    SLANG_FORCE_INLINE uint32_t CommittedInstanceID() const { return state.committed.instanceID; }
    SLANG_FORCE_INLINE uint32_t CandidatePrimitiveIndex() const
    {
        return state.candidate.primitiveIndex;
    }
    SLANG_FORCE_INLINE uint32_t CommittedPrimitiveIndex() const
    {
        return state.committed.primitiveIndex;
    }
    SLANG_FORCE_INLINE uint32_t CandidateGeometryIndex() const
    {
        return state.candidate.geometryIndex;
    }
    SLANG_FORCE_INLINE uint32_t CommittedGeometryIndex() const
    {
        return state.committed.geometryIndex;
    }

    SLANG_FORCE_INLINE float3 CandidateObjectRayOrigin() const
    {
        return _slangRayQueryGetFloat3(state.candidate.objectRayOrigin);
    }
    SLANG_FORCE_INLINE float3 CommittedObjectRayOrigin() const
    {
        return _slangRayQueryGetFloat3(state.committed.objectRayOrigin);
    }
    SLANG_FORCE_INLINE float3 CandidateObjectRayDirection() const
    {
        return _slangRayQueryGetFloat3(state.candidate.objectRayDirection);
    }
    SLANG_FORCE_INLINE float3 CommittedObjectRayDirection() const
    {
        return _slangRayQueryGetFloat3(state.committed.objectRayDirection);
    }
    SLANG_FORCE_INLINE bool CandidateTriangleFrontFace() const
    {
        return state.candidate.triangleFrontFace != 0;
    }
    SLANG_FORCE_INLINE bool CommittedTriangleFrontFace() const
    {
        return state.committed.triangleFrontFace != 0;
    }
    SLANG_FORCE_INLINE float2 CandidateTriangleBarycentrics() const
    {
        return _slangRayQueryGetFloat2(state.candidate.barycentrics);
    }
    SLANG_FORCE_INLINE float2 CommittedTriangleBarycentrics() const
    {
        return _slangRayQueryGetFloat2(state.committed.barycentrics);
    }

    SLANG_FORCE_INLINE Matrix<float, 3, 4> CandidateObjectToWorld3x4() const
    {
        return _slangRayQueryGetMatrix<3, 4>(state.candidate.objectToWorld, false);
    }
    SLANG_FORCE_INLINE Matrix<float, 3, 4> CommittedObjectToWorld3x4() const
    {
        return _slangRayQueryGetMatrix<3, 4>(state.committed.objectToWorld, false);
    }
    SLANG_FORCE_INLINE Matrix<float, 4, 3> CandidateObjectToWorld4x3() const
    {
        return _slangRayQueryGetMatrix<4, 3>(state.candidate.objectToWorld, true);
    }
    SLANG_FORCE_INLINE Matrix<float, 4, 3> CommittedObjectToWorld4x3() const
    {
        return _slangRayQueryGetMatrix<4, 3>(state.committed.objectToWorld, true);
    }
    SLANG_FORCE_INLINE Matrix<float, 3, 4> CandidateWorldToObject3x4() const
    {
        return _slangRayQueryGetMatrix<3, 4>(state.candidate.worldToObject, false);
    }
    SLANG_FORCE_INLINE Matrix<float, 3, 4> CommittedWorldToObject3x4() const
    {
        return _slangRayQueryGetMatrix<3, 4>(state.committed.worldToObject, false);
    }
    SLANG_FORCE_INLINE Matrix<float, 4, 3> CandidateWorldToObject4x3() const
    {
        return _slangRayQueryGetMatrix<4, 3>(state.candidate.worldToObject, true);
    }
    SLANG_FORCE_INLINE Matrix<float, 4, 3> CommittedWorldToObject4x3() const
    {
        return _slangRayQueryGetMatrix<4, 3>(state.committed.worldToObject, true);
    }

    SLANG_FORCE_INLINE uint32_t RayFlags() const { return state.rayFlags; }
    SLANG_FORCE_INLINE float3 WorldRayOrigin() const
    {
        return _slangRayQueryGetFloat3(state.worldRayOrigin);
    }
    SLANG_FORCE_INLINE float3 WorldRayDirection() const
    {
        return _slangRayQueryGetFloat3(state.worldRayDirection);
    }
    SLANG_FORCE_INLINE float RayTMin() const { return state.rayTMin; }

    RayQueryState state;
};

#endif
