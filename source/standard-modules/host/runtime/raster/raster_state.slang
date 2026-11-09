// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Raster pipeline policies contain only static members. Resources, clear values, viewport/scissor
// rectangles, and stencil reference belong to draw bindings, never to these policy types.
// Names and defaults follow slang-rhi/include/slang-rhi.h. Enum values here are source values;
// the compiler translates them to the stable HGBC schema instead of serializing RHI structures.

public enum FillMode
{
    Solid,
    Wireframe,
}

public enum CullMode
{
    None,
    Front,
    Back,
}

public enum FrontFaceMode
{
    CounterClockwise,
    Clockwise,
}

public enum ComparisonFunc
{
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
}

public enum StencilOp
{
    Keep,
    Zero,
    Replace,
    IncrementSaturate,
    DecrementSaturate,
    Invert,
    IncrementWrap,
    DecrementWrap,
}

public enum BlendOp
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
}

public enum BlendFactor
{
    Zero,
    One,
    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,
    DestAlpha,
    InvDestAlpha,
    DestColor,
    InvDestColor,
    SrcAlphaSaturate,
    BlendColor,
    InvBlendColor,
    SecondarySrcColor,
    InvSecondarySrcColor,
    SecondarySrcAlpha,
    InvSecondarySrcAlpha,
}

public enum LogicOp
{
    NoOp,
}

public enum RenderTargetWriteMask
{
    None = 0,
    Red = 1,
    Green = 2,
    Blue = 4,
    Alpha = 8,
    All = 15,
}

/// Supplies floating-point bias constants without floating-point value generic parameters.
public interface IRasterDepthBiasState
{
    static const float depthBiasClamp;
    static const float slopeScaledDepthBias;
}

public struct RasterDepthBiasState : IRasterDepthBiasState
{
    public static const float depthBiasClamp = 0.0;
    public static const float slopeScaledDepthBias = 0.0;
}

/// Configures rasterization independently of attachment formats and primitive topology.
public interface IRasterizerState
{
    static const FillMode fillMode;
    static const CullMode cullMode;
    static const FrontFaceMode frontFace;
    static const int depthBias;
    static const float depthBiasClamp;
    static const float slopeScaledDepthBias;
    static const bool depthClipEnable;
    static const bool scissorEnable;
    static const bool multisampleEnable;
    static const bool antialiasedLineEnable;
    static const bool enableConservativeRasterization;
    static const uint forcedSampleCount;
}

public struct RasterizerState<
    let Fill : FillMode = FillMode.Solid,
    let Cull : CullMode = CullMode.None,
    let FrontFace : FrontFaceMode = FrontFaceMode.CounterClockwise,
    let DepthBias : int = 0,
    let DepthClipEnable : bool = true,
    let ScissorEnable : bool = false,
    let MultisampleEnable : bool = false,
    let AntialiasedLineEnable : bool = false,
    let EnableConservativeRasterization : bool = false,
    let ForcedSampleCount : uint = 0,
    Bias : IRasterDepthBiasState = RasterDepthBiasState> : IRasterizerState
{
    public static const FillMode fillMode = Fill;
    public static const CullMode cullMode = Cull;
    public static const FrontFaceMode frontFace = FrontFace;
    public static const int depthBias = DepthBias;
    public static const float depthBiasClamp = Bias.depthBiasClamp;
    public static const float slopeScaledDepthBias = Bias.slopeScaledDepthBias;
    public static const bool depthClipEnable = DepthClipEnable;
    public static const bool scissorEnable = ScissorEnable;
    public static const bool multisampleEnable = MultisampleEnable;
    public static const bool antialiasedLineEnable = AntialiasedLineEnable;
    public static const bool enableConservativeRasterization = EnableConservativeRasterization;
    public static const uint forcedSampleCount = ForcedSampleCount;
}

/// Configures one stencil face; front and back faces may use different policies.
public interface IDepthStencilOpState
{
    static const StencilOp stencilFailOp;
    static const StencilOp stencilDepthFailOp;
    static const StencilOp stencilPassOp;
    static const ComparisonFunc stencilFunc;
}

public struct DepthStencilOpState<
    let Fail : StencilOp = StencilOp.Keep,
    let DepthFail : StencilOp = StencilOp.Keep,
    let Pass : StencilOp = StencilOp.Keep,
    let Compare : ComparisonFunc = ComparisonFunc.Always> : IDepthStencilOpState
{
    public static const StencilOp stencilFailOp = Fail;
    public static const StencilOp stencilDepthFailOp = DepthFail;
    public static const StencilOp stencilPassOp = Pass;
    public static const ComparisonFunc stencilFunc = Compare;
}

/// Keeps depth testing and writes independent. The depth format comes from the target type.
public interface IDepthStencilState
{
    static const bool depthTestEnable;
    static const bool depthWriteEnable;
    static const ComparisonFunc depthFunc;
    static const bool stencilEnable;
    static const uint stencilReadMask;
    static const uint stencilWriteMask;
    associatedtype FrontFace : IDepthStencilOpState;
    associatedtype BackFace : IDepthStencilOpState;
}

public struct DepthStencilState<
    let DepthTestEnable : bool = false,
    let DepthWriteEnable : bool = true,
    let DepthFunc : ComparisonFunc = ComparisonFunc.Less,
    let StencilEnable : bool = false,
    let StencilReadMask : uint = 0xFFFFFFFFu,
    let StencilWriteMask : uint = 0xFFFFFFFFu,
    Front : IDepthStencilOpState = DepthStencilOpState<>,
    Back : IDepthStencilOpState = DepthStencilOpState<>> : IDepthStencilState
{
    public static const bool depthTestEnable = DepthTestEnable;
    public static const bool depthWriteEnable = DepthWriteEnable;
    public static const ComparisonFunc depthFunc = DepthFunc;
    public static const bool stencilEnable = StencilEnable;
    public static const uint stencilReadMask = StencilReadMask;
    public static const uint stencilWriteMask = StencilWriteMask;
    public typealias FrontFace = Front;
    public typealias BackFace = Back;
}

/// Configures the blend equation for color or alpha independently.
public interface IAspectBlendState
{
    static const BlendFactor srcFactor;
    static const BlendFactor dstFactor;
    static const BlendOp op;
}

public struct AspectBlendState<
    let Source : BlendFactor = BlendFactor.One,
    let Destination : BlendFactor = BlendFactor.Zero,
    let Operation : BlendOp = BlendOp.Add> : IAspectBlendState
{
    public static const BlendFactor srcFactor = Source;
    public static const BlendFactor dstFactor = Destination;
    public static const BlendOp op = Operation;
}

/// Configures one ordered color slot. Its format is inferred from the bound texture type.
public interface IColorTargetState
{
    static const bool enableBlend;
    static const LogicOp logicOp;
    static const RenderTargetWriteMask writeMask;
    associatedtype Color : IAspectBlendState;
    associatedtype Alpha : IAspectBlendState;
}

public struct ColorTargetState<
    let EnableBlend : bool = false,
    let Logic : LogicOp = LogicOp.NoOp,
    let WriteMask : RenderTargetWriteMask = RenderTargetWriteMask.All,
    ColorBlend : IAspectBlendState = AspectBlendState<>,
    AlphaBlend : IAspectBlendState = AspectBlendState<>> : IColorTargetState
{
    public static const bool enableBlend = EnableBlend;
    public static const LogicOp logicOp = Logic;
    public static const RenderTargetWriteMask writeMask = WriteMask;
    public typealias Color = ColorBlend;
    public typealias Alpha = AlphaBlend;
}

/// Configures coverage. Attachment sample counts are inferred from texture types.
public interface IMultisampleState
{
    static const uint sampleMask;
    static const bool alphaToCoverageEnable;
    static const bool alphaToOneEnable;
}

public struct MultisampleState<
    let SampleMask : uint = 0xFFFFFFFFu,
    let AlphaToCoverageEnable : bool = false,
    let AlphaToOneEnable : bool = false> : IMultisampleState
{
    public static const uint sampleMask = SampleMask;
    public static const bool alphaToCoverageEnable = AlphaToCoverageEnable;
    public static const bool alphaToOneEnable = AlphaToOneEnable;
}

/// Preserves the ordered policy types for zero or more color outputs.
public interface IColorTargetStates
{
    associatedtype Targets;
    associatedtype Description;
    static Description describe();
}

public struct ColorTargetStates<each Target : IColorTargetState> : IColorTargetStates
{
    public typealias Targets = Tuple<Target>;
    public typealias Description = Tuple<expand __ColorStateDescription<each Target>.Data>;

    [ForceInline]
    public static Description describe()
    {
        return makeTuple(expand __ColorStateDescription<each Target>.get());
    }
}

// This normalized, constant-only description follows GraphicsColorTargetState in the HGBC
// schema. Keeping the source policy as a generic parameter allows ordinary specialization to
// fold all values before the compiler serializes the global pipeline definition.
public struct __ColorStateDescription<C : IColorTargetState>
{
    public typealias Data = uint[9];

    [ForceInline]
    public static Data get()
    {
        return {
            uint(C.enableBlend),
            uint(C.logicOp),
            uint(C.writeMask),
            uint(C.Color.srcFactor),
            uint(C.Color.dstFactor),
            uint(C.Color.op),
            uint(C.Alpha.srcFactor),
            uint(C.Alpha.dstFactor),
            uint(C.Alpha.op)
        };
    }
}

[ForceInline]
public Tuple<uint[5], float, float, uint[23]> __describeRasterState<
    State : IRasterState,
    let topology : uint>()
{
    typealias R = State.Rasterizer;
    typealias D = State.DepthStencil;
    typealias F = D.FrontFace;
    typealias B = D.BackFace;
    typealias M = State.Multisample;
    uint[5] header =
        { topology, uint(R.fillMode), uint(R.cullMode), uint(R.frontFace), uint(R.depthBias) };
    uint[23] tail = {
        uint(R.depthClipEnable),
        uint(R.scissorEnable),
        uint(R.multisampleEnable),
        uint(R.antialiasedLineEnable),
        uint(R.enableConservativeRasterization),
        R.forcedSampleCount,
        uint(D.depthTestEnable),
        uint(D.depthWriteEnable),
        uint(D.depthFunc),
        uint(D.stencilEnable),
        D.stencilReadMask,
        D.stencilWriteMask,
        uint(F.stencilFailOp),
        uint(F.stencilDepthFailOp),
        uint(F.stencilPassOp),
        uint(F.stencilFunc),
        uint(B.stencilFailOp),
        uint(B.stencilDepthFailOp),
        uint(B.stencilPassOp),
        uint(B.stencilFunc),
        M.sampleMask,
        uint(M.alphaToCoverageEnable),
        uint(M.alphaToOneEnable)
    };
    return makeTuple(header, R.depthBiasClamp, R.slopeScaledDepthBias, tail);
}

/// Supplies the independent state groups of a raster pipeline specialization.
public interface IRasterState
{
    associatedtype Rasterizer : IRasterizerState;
    associatedtype DepthStencil : IDepthStencilState;
    associatedtype ColorTargets : IColorTargetStates;
    associatedtype Multisample : IMultisampleState;
}

/// Packages independent state groups. ColorTargetStates<> describes a depth-only pipeline;
/// additional color policies describe independent blend equations for ordered output slots.
public struct RasterState<
    R : IRasterizerState = RasterizerState<>,
    D : IDepthStencilState = DepthStencilState<>,
    C : IColorTargetStates = ColorTargetStates<ColorTargetState<>>,
    M : IMultisampleState = MultisampleState<>> : IRasterState
{
    public typealias Rasterizer = R;
    public typealias DepthStencil = D;
    public typealias ColorTargets = C;
    public typealias Multisample = M;
}
