// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Hardware raytracing pipelines and their acceleration data are explicitly imported.
// Compute grids remain in the shared runtime; this module supplies dispatchRays.
import slang.runtime.compute.grid;

public interface ILogicalRay
{
    __init(float3 origin, float3 direction);
};

/// A parametric ray origin + t * direction with an interval [tMin,tMax]. Direction need
/// not be normalized; origin, direction, and interval endpoints support differentiation.
public struct Ray : ILogicalRay, IDifferentiable
{
    public float3 origin;
    public float3 direction;
    public float tMax;
    public float tMin;

    [Differentiable]
    public __init(float3 origin, float3 direction, float tMin, float tMax)
    {
        this.origin = origin;
        this.direction = direction;
        this.tMax = tMax;
        this.tMin = tMin;
    }

    [Differentiable]
    public __init(float3 origin, float3 direction)
    {
        this.origin = origin;
        this.direction = direction;
        this.tMax = 1e30;
        this.tMin = 0.0001;
    }

    [Differentiable]
    public Ray withLimits(float tMin, float tMax)
    {
        Ray r = Ray(this.origin, this.direction, tMin, tMax);
        return r;
    }

    public RayDesc toRayDesc()
    {
        return RayDesc(this.origin, this.tMin, this.direction, this.tMax);
    }
};

// One hardware type family is shared by every ray-type pack. Keep these definitions
// outside the generic context so aliases preserve identity across context specializations.
struct HWAccelerationTypes
{
    /// A graph-owned bottom-level acceleration structure shared by one or more scene instances.
    /// Allocation creates a logical handle; build inputs determine native storage requirements.
    __target_intrinsic(hgbc, "OpRaytracingBottomLevelAccelerationStructureType") [require(hgbc)]
    public struct BottomLevelAccelerationStructure
    {
        public static BottomLevelAccelerationStructure alloc()
        {
            return hgbc_asm
            {
            result:
                $$BottomLevelAccelerationStructure = OpAllocAccelerationStructure BottomLevel;
            };
        }

        /// Build an ordered list of procedural AABB geometries. Shape data is read separately
        /// by the intersection shader, indexed by PrimitiveIndex(). Build flags/modes match triangles.
        [require(hgbc)]
        public void build(
            RuntimeArray<AABBGeometry> geometries,
            uint buildFlags = 4,
            uint buildMode = 0)
        {
            hgbc_asm
            {
                OpBuildAccelerationStructure BottomLevel AABBs $$AABBGeometry $this $geometries
                    $buildFlags $buildMode;
            };
        }

        /// Build from an ordered list of triangle geometries. Flags use RHI build flag bits; the
        /// default 4 is PreferFastTrace. Mode 0 is Build; Update (1) is reserved and currently rejected.
        [require(hgbc)]
        public void build<VertexBuffer : IArray<float3>, IndexBuffer : IArray<uint>>(
            RuntimeArray<TriangleGeometryDesc<VertexBuffer, IndexBuffer>> geometries,
            uint buildFlags = 4,
            uint buildMode = 0)
        {
            hgbc_asm
            {
                OpBuildAccelerationStructure BottomLevel Triangles
                    $$TriangleGeometryDesc<VertexBuffer, IndexBuffer> $this $geometries $buildFlags
                        $buildMode;
            };
        }
    }

    /// One indexed triangle geometry in a bottom-level build, using float3 positions and uint indices.
    /// Offsets and counts are in buffer elements. Geometry flags use the RHI bit values: Opaque = 1
    /// and NoDuplicateAnyHitInvocation = 2. The initial default treats triangles as opaque.
    /// Vertex and index buffers may independently be StructuredBuffer or RWStructuredBuffer;
    /// the build reads either allocation without converting or copying its contents.
    [require(hgbc)]
    public struct TriangleGeometryDesc<VertexBuffer : IArray<float3>, IndexBuffer : IArray<uint>>
    {
        public VertexBuffer vertices;
        public IndexBuffer indices;
        public uint vertexOffset = 0;
        public uint vertexCount = 0;
        public uint indexOffset = 0;
        public uint indexCount = 0;
        public uint flags = 1;
    }

    /// The default triangle descriptor uses read-only buffers, preserving its original call syntax.
    typealias TriangleGeometry =
        TriangleGeometryDesc<StructuredBuffer<float3>, StructuredBuffer<uint>>;

    /// Packed native procedural bounds: six consecutive floats, in object space.
    /// The bounds only participate in traversal; an intersection shader reports actual hits.
    public struct AABB
    {
        public float minX;
        public float minY;
        public float minZ;
        public float maxX;
        public float maxY;
        public float maxZ;
    }

    /// One procedural geometry. Offsets/counts are in AABB elements. Set flags to 0 to enable
    /// any-hit callbacks; the default Opaque flag (1) suppresses them.
    [require(hgbc)]
    public struct AABBGeometry
    {
        public StructuredBuffer<AABB> bounds;
        public uint primitiveOffset = 0;
        public uint primitiveCount = 0;
        public uint flags = 1;
    }

    /// One instance of a built bottom-level acceleration structure in a top-level scene.
    /// Instance order and geometry counts determine shader-table contributions at dispatch time.
    /// `instanceID` is application data and does not choose a shader-table record.
    [require(hgbc)]
    public struct Instance
    {
        public BottomLevelAccelerationStructure blas;
        public float3x4 transform = float3x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0);
        public uint instanceID = 0;
        public uint mask = 255;
        public uint flags = 0;
    }
}

public extension RaytracingAccelerationStructure
{
    /// Allocate a logical top-level scene without guessing its native storage size.
    [require(hgbc)]
    public static RaytracingAccelerationStructure alloc()
    {
        return hgbc_asm
        {
        result:
            $$RaytracingAccelerationStructure = OpAllocAccelerationStructure TopLevel;
        };
    }

    /// Snapshot an ordered instance list. The native scene is realized at dispatch with the
    /// selected ray-type stride. Mode 0 is Build; Update (1) is reserved and currently rejected.
    [require(hgbc)]
    public void build(
        RuntimeArray<HWAccelerationTypes.Instance> instances,
        uint buildFlags = 4,
        uint buildMode = 0)
    {
        hgbc_asm
        {
            OpBuildAccelerationStructure TopLevel Instances $$HWAccelerationTypes.Instance $this
                $instances $buildFlags $buildMode;
        };
    }

    /// Return an owning bindless view of this logical scene. Native descriptor data is supplied
    /// when the scene is bound for a dispatch, after its shader-table stride is known.
    [require(hgbc)]
    public DescriptorHandle<RaytracingAccelerationStructure> getDescriptorHandle()
    {
        return hgbc_asm
        {
        result:
            $$DescriptorHandle<RaytracingAccelerationStructure> =
                OpResourceToDescriptorHandle $this;
        };
    }
}

/// Creates an independent registry without creating a native pipeline or shader binding table.
[require(hgbc)]
public RaytracingPipeline makeRaytracingPipeline(uint rayTypeCount)
{
    return hgbc_asm
    {
    result:
        $$RaytracingPipeline = OpMakeRaytracingPipeline $rayTypeCount;
    };
}

/// Returns this dispatch's offset, in records, for a captured ray-tracing registry.
[require(glsl_hlsl_spirv, raytracing_allstages)]
public uint getBindingOffset(RaytracingPipeline pipeline)
{
    return pipeline.getBindingOffset();
}

/// Builds a graph-owned ray-tracing scene containing one indexed triangle geometry.
///
/// The initial HGVM representation deliberately fixes the geometry formats to `float3` positions
/// at byte offset zero and `uint` indices. The counts are explicit so a caller may build from a
/// prefix of either buffer without the VM having to infer the intended geometry at run time.
__generic<VL : IBufferDataLayout, IL : IBufferDataLayout> [require(hgbc)]
public RaytracingAccelerationStructure createRaytracingScene(
    uint vertexCount,
    StructuredBuffer<float3, VL> vertices,
    uint indexCount,
    StructuredBuffer<uint, IL> indices)
{
    return hgbc_asm
    {
    result:
        $$RaytracingAccelerationStructure = OpCreateRaytracingScene IndexedTriangles Float3 UInt32
            $vertexCount $vertices $indexCount $indices;
    };
}

/// Builds a graph-owned ray-tracing scene from read-write geometry buffers.
__generic<VL : IBufferDataLayout, IL : IBufferDataLayout> [require(hgbc)]
public RaytracingAccelerationStructure createRaytracingScene(
    uint vertexCount,
    RWStructuredBuffer<float3, VL> vertices,
    uint indexCount,
    RWStructuredBuffer<uint, IL> indices)
{
    return hgbc_asm
    {
    result:
        $$RaytracingAccelerationStructure = OpCreateRaytracingScene IndexedTriangles Float3 UInt32
            $vertexCount $vertices $indexCount $indices;
    };
}

/// Marks a nominal ray category whose fields carry its state through traversal.
/// Distinct types select distinct shader records even when their fields have identical layouts.
public interface IRayType
{
}

/// Returns the unique nominal type's position, rejecting absent or repeated categories.
public uint rayTypeIndex<RayType, each RayTypes>()
{
    return uint(__index_of_type_in_pack<RayType, RayTypes>);
}

/// Supplies the native shader-stage access qualifiers for a complete ray state.
[raypayload]
struct RayPayload<RayType : IRayType>
{
    RayType value : read(caller, closesthit, anyhit, miss)
        : write(caller, closesthit, anyhit, miss);
}

[shader("miss")]
void contextMiss<RayType : IRayType, Func : IFunc<RayType, RayType>>(
    inout RayPayload<RayType> payload,
    uniform Func function)
{
    payload.value = function(payload.value);
}

[shader("closesthit")]
void contextClosestHit<RayType : IRayType, Attributes, Func : IFunc<RayType, RayType, Attributes>>(
    inout RayPayload<RayType> payload,
    in Attributes attributes,
    uniform Func function)
{
    payload.value = function(payload.value, attributes);
}

/// Controls traversal after an any-hit callback. Payload updates are published before the action.
public enum AnyHitAction
{
    Accept,
    Ignore,
    AcceptAndEndSearch,
}

public struct AnyHitResult<RayType : IRayType>
{
    public RayType payload;
    public AnyHitAction action = AnyHitAction.Accept;
}

/// Reports candidates from an intersection callback. A callback may report multiple hits, and
/// use the returned acceptance result to decide whether to try another candidate. Attributes
/// are passed through traversal to any-hit/closest-hit; they are separate from captured data.
public struct IntersectionReporter<Attributes>
{
    public bool report(float distance, Attributes attributes, uint hitKind = 0)
    {
        return ReportHit(distance, hitKind, attributes);
    }
}

/// Passes the object-space ray without normalizing its direction, preserving t under instance
/// scaling. tMax snapshots the current traversal bound when the callback begins. ReportHit still
/// tests candidates against the live bound, which accepted hits may shorten during the callback.
[shader("intersection")]
void contextIntersection<Attributes, Func : IFunc<void, Ray, IntersectionReporter<Attributes>>>(
    uniform Func function)
{
    Ray ray = Ray(ObjectRayOrigin(), ObjectRayDirection(), RayTMin(), RayTCurrent());
    function(ray, IntersectionReporter<Attributes>());
}

[shader("anyhit")]
void contextAnyHit<
    RayType : IRayType,
    Attributes,
    Func : IFunc<AnyHitResult<RayType>, RayType, Attributes>>(
    inout RayPayload<RayType> payload,
    in Attributes attributes,
    uniform Func function)
{
    let result = function(payload.value, attributes);
    payload.value = result.payload;
    if (result.action == AnyHitAction.Ignore)
        IgnoreHit();
    else if (result.action == AnyHitAction.AcceptAndEndSearch)
        AcceptHitAndEndSearch();
}

/// Type-level absence markers. Registration omits these stages from the native hit group.
public struct NoClosestHit<RayType : IRayType, Attributes> : IFunc<RayType, RayType, Attributes>
{
    public RayType operator()(RayType payload, Attributes attributes) { return payload; }
}

public struct NoAnyHit<RayType : IRayType, Attributes>
    : IFunc<AnyHitResult<RayType>, RayType, Attributes>
{
    public AnyHitResult<RayType> operator()(RayType payload, Attributes attributes)
    {
        return AnyHitResult<RayType>(payload, AnyHitAction.Accept);
    }
}

/// A procedural group has a required intersection callback and optional shading/filter callbacks.
/// The intersection signature is (Ray ray, IntersectionReporter<Attributes> reporter) -> void.
/// Use makeProceduralHitGroup<RayType, Attributes>() to infer the callback types.
/// Shader code and stage presence are static. Captured values are snapshotted by onHit().
public struct ProceduralHitGroup<
    RayType : IRayType,
    Attributes,
    Intersection : IFunc<void, Ray, IntersectionReporter<Attributes>>,
    ClosestHit : IFunc<RayType, RayType, Attributes> = NoClosestHit<RayType, Attributes>,
    AnyHit : IFunc<AnyHitResult<RayType>, RayType, Attributes> = NoAnyHit<RayType, Attributes>>
{
    Intersection intersection;
    ClosestHit closestHit;
    AnyHit anyHit;

    public __init(Intersection intersection, ClosestHit closestHit = {}, AnyHit anyHit = {})
    {
        this.intersection = intersection;
        this.closestHit = closestHit;
        this.anyHit = anyHit;
    }

    [host]
    void register(RaytracingPipeline pipeline, uint geometrySlot, uint raySlot)
    {
        if (__type_equals<ClosestHit, NoClosestHit<RayType, Attributes>>())
        {
            if (__type_equals<AnyHit, NoAnyHit<RayType, Attributes>>())
                __setRayHitGroup(
                    pipeline,
                    geometrySlot,
                    raySlot,
                    RayPayload<RayType>,
                    Attributes,
                    void,
                    void,
                    contextIntersection<Attributes, Intersection>)(intersection);
            else
                __setRayHitGroup(
                    pipeline,
                    geometrySlot,
                    raySlot,
                    RayPayload<RayType>,
                    Attributes,
                    void,
                    contextAnyHit<RayType, Attributes, AnyHit>,
                    contextIntersection<Attributes, Intersection>)(anyHit, intersection);
        }
        else
        {
            if (__type_equals<AnyHit, NoAnyHit<RayType, Attributes>>())
                __setRayHitGroup(
                    pipeline,
                    geometrySlot,
                    raySlot,
                    RayPayload<RayType>,
                    Attributes,
                    contextClosestHit<RayType, Attributes, ClosestHit>,
                    void,
                    contextIntersection<Attributes, Intersection>)(closestHit, intersection);
            else
                __setRayHitGroup(
                    pipeline,
                    geometrySlot,
                    raySlot,
                    RayPayload<RayType>,
                    Attributes,
                    contextClosestHit<RayType, Attributes, ClosestHit>,
                    contextAnyHit<RayType, Attributes, AnyHit>,
                    contextIntersection<Attributes, Intersection>)(
                    closestHit,
                    anyHit,
                    intersection);
        }
    }
}

/// Construct a typed procedural group while inferring the closure types from its arguments.
public ProceduralHitGroup<RayType, Attributes, Intersection, ClosestHit, AnyHit> makeProceduralHitGroup<
    RayType : IRayType,
    Attributes,
    Intersection : IFunc<void, Ray, IntersectionReporter<Attributes>>,
    ClosestHit : IFunc<RayType, RayType, Attributes> = NoClosestHit<RayType, Attributes>,
    AnyHit : IFunc<AnyHitResult<RayType>, RayType, Attributes> = NoAnyHit<RayType, Attributes>>(
    Intersection intersection,
    ClosestHit closestHit = {},
    AnyHit anyHit = {})
{
    return ProceduralHitGroup<RayType, Attributes, Intersection, ClosestHit, AnyHit>(
        intersection,
        closestHit,
        anyHit);
}

/// Shares deferred miss/hit registrations. Copying a context aliases the same host tracker.
/// Dispatch currently supports one tracker, triangle groups, and procedural hit groups.
/// Callback resources must be explicit descriptor handles; ordinary resource captures are rejected.
public struct HWRaytracingContext<each RayTypes : IRayType>
{
    /// Hardware acceleration data is independent of the registered ray types. For example,
    /// HWRaytracingContext<Primary>.Instance and HWRaytracingContext<Primary, Shadow>.Instance
    /// are the same type, allowing both contexts to trace the same scene.
    /// HWRaytracingContext<> exposes this type family without constructing a context.
    public typealias AccelerationStructure = RaytracingAccelerationStructure;
    public typealias BottomLevelAccelerationStructure =
        HWAccelerationTypes.BottomLevelAccelerationStructure;
    public typealias
        TriangleGeometryDesc<VertexBuffer : IArray<float3>, IndexBuffer : IArray<uint>> =
            HWAccelerationTypes.TriangleGeometryDesc<VertexBuffer, IndexBuffer>;
    public typealias TriangleGeometry = HWAccelerationTypes.TriangleGeometry;
    public typealias AABB = HWAccelerationTypes.AABB;
    public typealias AABBGeometry = HWAccelerationTypes.AABBGeometry;
    public typealias Instance = HWAccelerationTypes.Instance;

    private RaytracingPipeline pipeline;

    [host]
    public __init()
    {
        static_assert(
            countof(RayTypes) > 0 && countof(RayTypes) <= 15,
            "A context requires between one and fifteen ray types.");
        expand rayTypeIndex<each RayTypes, RayTypes>();
        pipeline = makeRaytracingPipeline(countof(RayTypes));
    }

    /// Replaces a ray type's miss callback, retaining a snapshot of its captured values.
    [host]
    public void onMiss<RayType : IRayType, Func : IFunc<RayType, RayType>>(Func function)
    {
        __setRayMiss(pipeline, rayTypeIndex<RayType, RayTypes>(), contextMiss<RayType, Func>)(
            function);
    }

    /// Replaces one geometry/ray pair's closest-hit callback. Geometry slots follow TLAS
    /// instance order, with each instance contributing its BLAS geometries in build order.
    [host]
    public void onHit<
        RayType : IRayType,
        Func : IFunc<RayType, RayType, BuiltInTriangleIntersectionAttributes>>(
        uint geometrySlot,
        Func function)
    {
        __setRayHitShader(
            pipeline,
            geometrySlot,
            rayTypeIndex<RayType, RayTypes>(),
            contextClosestHit<RayType, BuiltInTriangleIntersectionAttributes, Func>)(function);
    }

    /// Replaces a complete procedural group for one geometry/ray pair.
    [host]
    public void onHit<
        RayType : IRayType,
        Attributes,
        Intersection : IFunc<void, Ray, IntersectionReporter<Attributes>>,
        ClosestHit : IFunc<RayType, RayType, Attributes>,
        AnyHit : IFunc<AnyHitResult<RayType>, RayType, Attributes>>(
        uint geometrySlot,
        ProceduralHitGroup<RayType, Attributes, Intersection, ClosestHit, AnyHit> group)
    {
        group.register(pipeline, geometrySlot, rayTypeIndex<RayType, RayTypes>());
    }

    /// Traces against the supplied scene, updating the ray state in place.
    /// The state type selects the shader records; the context does not own the scene.
    public void traceRay<RayType : IRayType>(
        AccelerationStructure scene,
        RayDesc ray,
        inout RayType payload,
        uint flags = 0,
        uint instanceMask = 0xff)
    {
        uint column = getBindingOffset(pipeline) + rayTypeIndex<RayType, RayTypes>();
        RayPayload<RayType> nativePayload = { payload };
        // TODO: Multiple trackers require the dispatch-wide stride, not this context's count.
        // HGVM currently rejects a dispatch containing a second distinct tracker.
        TraceRay(scene, flags, instanceMask, column, countof(RayTypes), column, ray, nativePayload);
        payload = nativePayload.value;
    }
}

[shader("raygeneration")]
void dispatchGridRayGeneration<Index : IGridIndex, Func : IFunc<void, Index>>(uniform Func function)
{
    function(Index.fromGridIndex(DispatchRaysIndex()));
}

public extension<Index : IGridIndex> DispatchGrid<Index>
{
    /// Enqueues one ray-generation invocation per coordinate, without compute workgroups.
    /// Recursion depth is a generic pipeline setting; callback values and extent are runtime data.
    [host]
    public void dispatchRays<let MaxRecursion : uint = 1, Func : IFunc<void, Index>>(Func function)
    {
        __dispatchRays(dispatchGridRayGeneration<Index, Func>, size.gridExtent(), MaxRecursion)(
            function);
    }
}
