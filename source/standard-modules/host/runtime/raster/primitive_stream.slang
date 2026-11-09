// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.core.simple_assert;
import slang.runtime.raster.raster_state;
import slang.runtime.raster.render_targets;
import slang.runtime.raster.draw_parameters;

/// Describes how ordered vertex references are assembled, independently of their storage.
public enum Topology
{
    TriangleList = 0,
    PointList = 1,
    LineList = 2,
    LineStrip = 3,
    TriangleStrip = 4,
    PatchList = 5,
}

/// Effective draw IDs, including the draw's first vertex/base vertex and first instance.
public struct VertexInvocation
{
    public uint vertexID;
    public uint instanceID;
}

/// A primitive source counts vertex references, not assembled primitives or shader executions.
public interface IPrimitiveSource
{
    uint getCount();
    uint getVertexOffset();
    __GraphicsDraw assemble(uint instanceCount, uint firstInstance);
}

/// Sequential vertex IDs. A vertex callback may generate all attributes without a vertex buffer.
public struct VertexRange : IPrimitiveSource
{
    public uint vertexCount;
    public uint firstVertex;

    [host]
    public __init(uint vertexCount, uint firstVertex = 0)
    {
        assert(vertexCount == 0 || firstVertex <= 0xffffffffu - (vertexCount - 1));
        this.vertexCount = vertexCount;
        this.firstVertex = firstVertex;
    }

    public uint getCount() { return vertexCount; }
    public uint getVertexOffset() { return firstVertex; }

    [host]
    public __GraphicsDraw assemble(uint instanceCount, uint firstInstance)
    {
        int baseVertex = 0;
        return hgbc_asm
        {
        result:
            $$__GraphicsDraw = OpMakeGraphicsDraw $vertexCount $firstVertex $instanceCount
                $firstInstance $baseVertex;
        };
    }
}

/// A range in a typed index buffer. The buffer type remains the source of truth for index width.
/// Construction validates host-known ranges without reading index contents back from the GPU.
public struct IndexedRange<Buffer> : IPrimitiveSource
{
    public Buffer indices;
    public uint firstIndex;
    public uint indexCount;
    public int baseVertex;

    public uint getCount() { return indexCount; }
    public uint getVertexOffset() { return uint(baseVertex); }

    [host]
    public __GraphicsDraw assemble(uint instanceCount, uint firstInstance)
    {
        return hgbc_asm
        {
        result:
            $$__GraphicsDraw = OpMakeGraphicsDraw $indexCount $firstIndex $instanceCount
                $firstInstance $baseVertex $indices;
        };
    }
}

/// Retains a source and an instance range without recording GPU work.
public struct PrimitiveStream<Source : IPrimitiveSource, let AssemblyTopology : Topology>
{
    public Source source;
    public uint instanceCount;
    public uint firstInstance;
    public static const Topology topology = AssemblyTopology;

    [host]
    public This instances(uint count, uint first = 0)
    {
        assert(count == 0 || first <= 0xffffffffu - (count - 1));
        return { source, count, first };
    }

    /// Attaches a callback type and its captured values; it does not run the callback.
    public VertexPrimitiveStream<Source, AssemblyTopology, Varyings, Vertex> vertex<
        Varyings,
        Vertex : IFunc<Varyings, VertexInvocation>>(Vertex function)
    {
        return { this, function };
    }
}

/// A deferred classic vertex-stage recipe. Other stage families can share the downstream draw API
/// without fabricating a vertex callback or vertex-buffer resource.
public struct VertexPrimitiveStream<
    Source : IPrimitiveSource,
    let AssemblyTopology : Topology,
    Varyings,
    Vertex : IFunc<Varyings, VertexInvocation>>
{
    public PrimitiveStream<Source, AssemblyTopology> primitives;
    public Vertex vertexShader;

    [host]
    public This instances(uint count, uint first = 0)
    {
        return { primitives.instances(count, first), vertexShader };
    }

    [host]
    public void draw<
        State : IRasterState = RasterState<>,
        Color : IRenderColorTarget,
        Fragment : IFunc<Color.ClearValue, Varyings>>(
        RenderTargetSet<Tuple<Color>, NoDepthTarget> targets,
        Fragment fragment,
        DrawParameters parameters = {})
    {
        let description = __graphicsDescriptor<Color, NoDepthTarget, State, false>(makeTuple(
            __describeRasterState<State, uint(AssemblyTopology)>(),
            State.ColorTargets.describe()));
        __makeGraphicsPipeline(
            __primitiveVertex<Varyings, Vertex>,
            __primitiveFragment<Varyings, Color.ClearValue, Fragment>,
            description)(
            targets.colors._0,
            primitives.source.assemble(primitives.instanceCount, primitives.firstInstance),
            parameters.lower(),
            __VertexBindings<Vertex>(
                vertexShader,
                primitives.source.getVertexOffset(),
                primitives.firstInstance),
            fragment);
    }

    [host]
    public void draw<
        State : IRasterState = RasterState<>,
        Color : IRenderColorTarget,
        let DepthFormat : int,
        Fragment : IFunc<Color.ClearValue, Varyings>>(
        RenderTargetSet<Tuple<Color>, Texture2D<float, 0, DepthFormat>> targets,
        Fragment fragment,
        DrawParameters parameters = {})
    {
        let description =
            __graphicsDescriptor<Color, Texture2D<float, 0, DepthFormat>, State, true>(makeTuple(
                __describeRasterState<State, uint(AssemblyTopology)>(),
                State.ColorTargets.describe()));
        __makeGraphicsPipeline(
            __primitiveVertex<Varyings, Vertex>,
            __primitiveFragment<Varyings, Color.ClearValue, Fragment>,
            description)(
            targets.colors._0,
            targets.depth,
            primitives.source.assemble(primitives.instanceCount, primitives.firstInstance),
            parameters.lower(),
            __VertexBindings<Vertex>(
                vertexShader,
                primitives.source.getVertexOffset(),
                primitives.firstInstance),
            fragment);
    }
}

[ForceInline]
__GraphicsPipelineDesc<
    Color,
    __GraphicsDraw,
    __GraphicsDrawParameters,
    State,
    HasDepth,
    Data,
    Depth> __graphicsDescriptor<Color, Depth, State, let HasDepth : bool, Data>(Data data)
{
    return { data };
}

struct __VertexBindings<Vertex>
{
    Vertex shader;
    uint vertexOffset;
    uint firstInstance;
}

[shader("vertex")]
Varyings __primitiveVertex<Varyings, Vertex : IFunc<Varyings, VertexInvocation>>(
    uint vertexID: SV_VertexID,
    uint instanceID: SV_InstanceID,
    uniform __VertexBindings<Vertex> bindings)
{
    // These Slang system values exclude the base offsets on the shader-pulling path. The domain
    // exposes effective IDs, so the adapter adds the source offset and first instance once.
    return bindings.shader(
        VertexInvocation(vertexID + bindings.vertexOffset, instanceID + bindings.firstInstance));
}

[shader("fragment")]
Output __primitiveFragment<Varyings, Output, Fragment : IFunc<Output, Varyings>>(
    Varyings varyings,
    uniform Fragment fragment)
    : SV_Target0
{
    return fragment(varyings);
}

public PrimitiveStream<Source, AssemblyTopology> Primitives<
    let AssemblyTopology : Topology,
    Source : IPrimitiveSource>(Source source)
{
    return { source, 1, 0 };
}

public IndexedRange<StructuredBuffer<uint, Layout>> IndexRange<Layout : IBufferDataLayout>(
    StructuredBuffer<uint, Layout> indices,
    uint firstIndex,
    uint indexCount,
    int baseVertex = 0)
{
    uint count;
    uint stride;
    indices.GetDimensions(count, stride);
    assert(firstIndex <= count && indexCount <= count - firstIndex);
    return { indices, firstIndex, indexCount, baseVertex };
}

public IndexedRange<StructuredBuffer<uint, Layout>> IndexRange<Layout : IBufferDataLayout>(
    StructuredBuffer<uint, Layout> indices)
{
    uint count;
    uint stride;
    indices.GetDimensions(count, stride);
    return { indices, 0, count, 0 };
}

public IndexedRange<RWStructuredBuffer<uint, Layout>> IndexRange<Layout : IBufferDataLayout>(
    RWStructuredBuffer<uint, Layout> indices,
    uint firstIndex,
    uint indexCount,
    int baseVertex = 0)
{
    uint count;
    uint stride;
    indices.GetDimensions(count, stride);
    assert(firstIndex <= count && indexCount <= count - firstIndex);
    return { indices, firstIndex, indexCount, baseVertex };
}

public IndexedRange<RWStructuredBuffer<uint, Layout>> IndexRange<Layout : IBufferDataLayout>(
    RWStructuredBuffer<uint, Layout> indices)
{
    uint count;
    uint stride;
    indices.GetDimensions(count, stride);
    return { indices, 0, count, 0 };
}
