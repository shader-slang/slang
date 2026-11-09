// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm):
// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.runtime.raster.raster_state;
import slang.runtime.core.simple_assert;

// Resolve policy members through interface witnesses and associated types, as the draw adapter
// will do. No policy value is constructed or passed to this function.
[host]
void checkDefaults<State : IRasterState>()
{
    assert(State.Rasterizer.fillMode == FillMode.Solid);
    assert(State.Rasterizer.cullMode == CullMode.None);
    assert(State.Rasterizer.frontFace == FrontFaceMode.CounterClockwise);
    assert(State.Rasterizer.depthBias == 0);
    assert(State.Rasterizer.depthBiasClamp == 0.0);
    assert(State.Rasterizer.slopeScaledDepthBias == 0.0);
    assert(State.Rasterizer.depthClipEnable);
    assert(!State.Rasterizer.scissorEnable);
    assert(!State.Rasterizer.multisampleEnable);
    assert(!State.Rasterizer.antialiasedLineEnable);
    assert(!State.Rasterizer.enableConservativeRasterization);
    assert(State.Rasterizer.forcedSampleCount == 0);
    assert(!State.DepthStencil.depthTestEnable);
    assert(State.DepthStencil.depthWriteEnable);
    assert(State.DepthStencil.depthFunc == ComparisonFunc.Less);
    assert(!State.DepthStencil.stencilEnable);
    assert(State.DepthStencil.stencilReadMask == 0xffffffffu);
    assert(State.DepthStencil.stencilWriteMask == 0xffffffffu);
    assert(State.DepthStencil.FrontFace.stencilFailOp == StencilOp.Keep);
    assert(State.DepthStencil.BackFace.stencilFunc == ComparisonFunc.Always);
    assert(State.Multisample.sampleMask == 0xffffffffu);
    assert(!State.Multisample.alphaToCoverageEnable);
    assert(!State.Multisample.alphaToOneEnable);
}

struct FractionalBias : IRasterDepthBiasState
{
    static const float depthBiasClamp = 0.25;
    static const float slopeScaledDepthBias = -1.5;
}

typealias BiasedRasterizer = RasterizerState<
    FillMode.Wireframe,
    CullMode.Back,
    FrontFaceMode.Clockwise,
    -3,
    false,
    true,
    true,
    true,
    true,
    4,
    FractionalBias>;
typealias FrontStencil = DepthStencilOpState<
    StencilOp.Replace,
    StencilOp.IncrementSaturate,
    StencilOp.DecrementWrap,
    ComparisonFunc.GreaterEqual>;
typealias BackStencil = DepthStencilOpState<
    StencilOp.Invert,
    StencilOp.DecrementSaturate,
    StencilOp.IncrementWrap,
    ComparisonFunc.NotEqual>;
typealias Stencil = DepthStencilState<
    true,
    false,
    ComparisonFunc.Equal,
    true,
    0x7fu,
    0x3fu,
    FrontStencil,
    BackStencil>;
typealias ColorBlend = AspectBlendState<BlendFactor.SrcAlpha, BlendFactor.InvSrcAlpha, BlendOp.Add>;
typealias AlphaBlend = AspectBlendState<BlendFactor.One, BlendFactor.One, BlendOp.Max>;
typealias BlendedTarget = ColorTargetState < true, LogicOp.NoOp,
          RenderTargetWriteMask.Red | RenderTargetWriteMask.Alpha, ColorBlend, AlphaBlend > ;
typealias TwoTargets = ColorTargetStates<BlendedTarget, ColorTargetState<>>;
typealias CustomState =
    RasterState<BiasedRasterizer, Stencil, TwoTargets, MultisampleState<0x55u, true, true>>;

[host]
void checkCustom<State : IRasterState>()
{
    assert(State.Rasterizer.depthBias == -3);
    assert(State.Rasterizer.depthBiasClamp == 0.25);
    assert(State.Rasterizer.slopeScaledDepthBias == -1.5);
    assert(State.Rasterizer.scissorEnable);
    assert(State.DepthStencil.depthTestEnable && !State.DepthStencil.depthWriteEnable);
    assert(State.DepthStencil.FrontFace.stencilPassOp == StencilOp.DecrementWrap);
    assert(State.DepthStencil.BackFace.stencilPassOp == StencilOp.IncrementWrap);
    assert(State.DepthStencil.stencilReadMask == 0x7fu);
    assert(State.DepthStencil.stencilWriteMask == 0x3fu);
    assert(State.Multisample.sampleMask == 0x55u);
}

[host]
void checkTargetTypes<States : IColorTargetStates, Expected>()
    where States.Targets == Expected {}

[host]
void main()
{
    checkDefaults<RasterState<>>();
    checkCustom<CustomState>();
    assert(BlendedTarget.Color.srcFactor == BlendFactor.SrcAlpha);
    assert(BlendedTarget.Alpha.op == BlendOp.Max);
    assert(BlendedTarget.writeMask == (RenderTargetWriteMask.Red | RenderTargetWriteMask.Alpha));
    // The color policy list supports zero and multiple slots independently of resource bindings.
    checkTargetTypes<TwoTargets, Tuple<BlendedTarget, ColorTargetState<>>>();
    checkTargetTypes<ColorTargetStates<>, Tuple<>>();
}

// Specialized policies do not become runtime arguments or constructed raster-state values.
// CHECK: Section("FUNC")
// CHECK-NOT: OpParam
// CHECK-NOT: OpMakeStruct
