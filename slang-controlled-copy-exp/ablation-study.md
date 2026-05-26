# Issue 11085 Hand-Tuning Ablation Study

This note tracks the controlled hand-tuning experiments for the Slang issue 11085
minimal repro. The goal is to identify the smallest compiler-side codegen changes
that let Apple's Metal optimizer remove the large aggregate copies and stack
temporaries from the AD-generated backward chain.

## Files In This Folder

- `issue-11085-minimal-repro.slang`
  - Cleaned original repro source.
  - Uses AD synthesis for `outer`.
  - Keeps the custom backward derivative for `inner`.
  - Does not contain the optional custom `outerBwd` path.

- `issue-11085-default-cleaned.hlsl`
  - HLSL emitted from the cleaned repro.
  - This is the AD-generated baseline.

- `issue-11085-default-forceinline.hlsl`
  - Current working HLSL experiment file.
  - It has the AD chain helpers force-inlined, read-only aggregate parameters
    converted to `__constref`, and `_S14` constructed by field assignment.
  - As of the latest experiment, `BigVec_x24_syn_dzero_0` and
    `BigVec_x24_syn_dadd_0` intentionally do not have `[ForceInline]`.

- `issue-11085-default-hlsl-hand-tuned.slang`
  - Compact hand-tuned Slang form derived from the AD-generated HLSL.
  - Uses loops instead of the fully unrolled HLSL body.
  - Represents the no-`memcpy`, no-`alloca` target shape.

## What We Count

All counts below are from Metal LLVM emitted by:

```sh
build/Release/bin/slangc <input> -target metal -entry computeMain -stage compute -o <output>.metal
xcrun -sdk macosx metal -std=metal3.0 -O3 -S -emit-llvm -c <output>.metal -o <output>.ll
```

Counters:

```sh
rg -c "call void @llvm\\.memcpy" <output>.ll
rg -c "call void @llvm\\.memcpy.*i64 512|i64 512.*call void @llvm\\.memcpy" <output>.ll
rg -c "alloca" <output>.ll
```

The first counter intentionally counts only actual `llvm.memcpy` calls, not the
`llvm.memcpy` declaration.

## Baseline Shape

The AD-generated HLSL baseline has this key shape:

```hlsl
void s_bwdProp_outer_0(s_bwdCallableCtx_outer_0 _S3, out BigVec_0 _S4, BigVec_0 _S5)
{
    s_bwdCallableCtx_inner_0 _S6;
    _S6._S1 = _S3.value0_1.value0_0;

    BigVec_0 _S7 = BigVec_x24_syn_dzero_0();
    float _S8[int(128)] = { 0.0f, ... };
    BigVec_0 _S9;
    _S9.data_0 = _S8;

    s_bwdProp_inner_0(_S6, _S9, _S5);
    _S4 = BigVec_x24_syn_dadd_0(_S9, _S7);
}

void _S10(inout DiffPair_BigVec_0 _S11, BigVec_0 _S12)
{
    s_paramCtx_s_bwdCallableCtx_outer_0 _S13;
    _S13.value0_0 = _S11.primal_0;
    s_bwdCallableCtx_outer_0 _S14 = { _S13 };
    s_bwdProp_outer_0(_S14, _S11.differential_0, _S12);
}
```

The important suspected copy sources are:

- by-value aggregate parameters in the backward chain,
- non-inlined aggregate helper calls,
- the aggregate initializer `s_bwdCallableCtx_outer_0 _S14 = { _S13 }`,
- synthesized differential helpers `BigVec_x24_syn_dzero_0` and
  `BigVec_x24_syn_dadd_0`.

## Ablation Results

| Variant | Source Shape | memcpy calls | 512-byte memcpy calls | alloca | Notes |
|---|---|---:|---:|---:|---|
| A. AD baseline | `issue-11085-default-cleaned.hlsl` unchanged | 4 | 4 | 8 | Large copies and stack slots remain. |
| B. ForceInline only | Add `[ForceInline]` to `innerBwd_0`, `s_bwdProp_inner_0`, `dzero`, `dadd`, `s_bwdProp_outer_0`, and `_S10`; keep by-value params and aggregate init | 0 | 0 | 1 | Inlining removes the large copies. One stack slot remains. |
| C. ForceInline + `__constref` | Same as B, but read-only aggregate params are `__constref`; aggregate init remains | 0 | 0 | 1 | `__constref` does not remove the remaining stack slot. |
| D. ForceInline + field assignment, by-value params | Same as B, but replace aggregate init with direct field assignment | 0 | 0 | 0 | Shows `__constref` is not required when full inlining succeeds. |
| E. ForceInline + `__constref` + field assignment | Same as C, plus direct field assignment | 0 | 0 | 0 | This is the best HLSL shape. |
| I. ForceInline + `in` + field assignment | Same as E, but replace `__constref` read-only params with explicit `in` params | 0 | 0 | 0 | Full inlining still lets Metal eliminate the aggregate traffic. |
| F. Remove ForceInline from `dzero`/`dadd` | Same as E, but `dzero`/`dadd` are no longer `[ForceInline]` | 0 | 0 | 2 | Copies stay gone, but stack slots come back for addressable `BigVec` call operands. |
| G. Hand-tuned Slang | `issue-11085-default-hlsl-hand-tuned.slang` | 0 | 0 | 0 | Compact Slang version of the target shape. |
| H. ForceInline only return-value helpers | Same as E, but remove `[ForceInline]` from `void` helpers and keep it only on `dzero`/`dadd` | 5 | 5 | 6 | The non-inlined `void` AD wrappers reintroduce large aggregate copies. |

## Important Deltas

### 1. ForceInline on the AD Chain

Adding `[ForceInline]` to the backward chain is the first major improvement.
It removes all observed `llvm.memcpy` calls in this repro.

Functions marked in the experiment:

```hlsl
[ForceInline] void innerBwd_0(...)
[ForceInline] void s_bwdProp_inner_0(...)
[ForceInline] BigVec_0 BigVec_x24_syn_dzero_0()
[ForceInline] BigVec_0 BigVec_x24_syn_dadd_0(...)
[ForceInline] void s_bwdProp_outer_0(...)
[ForceInline] void _S10(...)
```

After this change, Metal LLVM no longer needs the 512-byte copies, but it still
emits:

```llvm
%2 = alloca %struct.s_paramCtx_s_bwdCallableCtx_outer_0_0
%132 = call %struct.s_bwdCallableCtx_outer_0_0
    @_Z34s_bwdCallableCtx_outer_0_x24init_0(...)
```

This points at the aggregate initializer, not at the differential add/zero
helpers.

### 2. `__constref` Is Helpful But Not Sufficient

Changing read-only aggregate parameters to `__constref` keeps the result at:

```text
memcpy=0
alloca=1
```

So `__constref` is not the fix for the remaining alloca. It is still useful
because it prevents read-only by-value aggregate parameter copies when a call is
not fully inlined, but it does not let the caller avoid materializing an
addressable aggregate if a non-inlined callee needs an address.

### 3. Aggregate Init Is The Remaining Alloca Source

This baseline code:

```hlsl
s_bwdCallableCtx_outer_0 _S14 = { _S13 };
```

lowers through a generated initializer helper:

```llvm
@_Z34s_bwdCallableCtx_outer_0_x24init_0(...)
```

Replacing it with field assignment removes the helper and the final stack slot:

```hlsl
s_bwdCallableCtx_outer_0 _S14;
_S14.value0_1 = _S13;
```

This was enough to move the HLSL experiment from:

```text
memcpy=0, alloca=1
```

to:

```text
memcpy=0, alloca=0
```

### 4. `dzero` And `dadd` Still Need ForceInline For alloca=0

When `BigVec_x24_syn_dzero_0` and `BigVec_x24_syn_dadd_0` are not forced inline,
the optimized Metal LLVM keeps `dadd` as a separate aggregate function:

```llvm
define %struct.BigVec_0_0 @_Z23BigVec_x24_syn_dadd_0_0PK10BigVec_0_0S1_(...)
```

Then `computeMain` must materialize addressable operands:

```llvm
%2 = alloca %struct.BigVec_0_0
%3 = alloca %struct.BigVec_0_0
%262 = call %struct.BigVec_0_0 @_Z23BigVec_x24_syn_dadd_0_0PK10BigVec_0_0S1_(...)
```

This variant has:

```text
memcpy=0
alloca=2
```

So `__constref` avoids by-value copies, but it does not avoid stack materialization
when the non-inlined function needs pointer operands.

### 5. Return-Value Helpers Alone Are Not Enough

We also tried the inverse experiment: keep `[ForceInline]` only on the functions
that return `BigVec_0`, and remove it from all `void` functions:

```hlsl
void innerBwd_0(...)
void s_bwdProp_inner_0(...)
[ForceInline] BigVec_0 BigVec_x24_syn_dzero_0()
[ForceInline] BigVec_0 BigVec_x24_syn_dadd_0(...)
void s_bwdProp_outer_0(...)
void _S10(...)
```

This produced:

```text
memcpy=5
alloca=6
```

The return-value helpers were no longer visible as separate LLVM functions, but
the `void` AD wrappers remained as calls:

```llvm
define void @_Z19s_bwdProp_inner_0_0(...)
define void @_Z19s_bwdProp_outer_0_0(...)
define void @_Z7U_S10_0(...)
```

Those wrappers pass large aggregate state through `out`/`inout` pointer
parameters, and that is enough to bring the 512-byte copies back. So the problem
is not just that Metal fails to optimize aggregate return values. In this repro,
the `void` wrapper calls are also an essential barrier.

## Minimal Source-Level Shape That Works

For this repro, the minimal observed no-copy/no-stack shape is:

1. Force-inline the AD backward chain and synthesized differential helpers.
2. Avoid aggregate initializer syntax for the backward callable context.
3. Prefer direct field assignment for context construction.

The critical `_S10` shape is:

```hlsl
[ForceInline]
void _S10(inout DiffPair_BigVec_0 _S11, BigVec_0 _S12)
{
    s_paramCtx_s_bwdCallableCtx_outer_0 _S13;
    _S13.value0_0 = _S11.primal_0;

    s_bwdCallableCtx_outer_0 _S14;
    _S14.value0_1 = _S13;

    s_bwdProp_outer_0(_S14, _S11.differential_0, _S12);
}
```

The `__constref` version is also good, and likely more robust if inlining fails
somewhere:

```hlsl
[ForceInline]
void s_bwdProp_outer_0(
    __constref s_bwdCallableCtx_outer_0 _S3,
    out BigVec_0 _S4,
    __constref BigVec_0 _S5);
```

But in this exact repro, full inlining plus field assignment was enough even with
by-value signatures.

## `in` vs `__constref`

We restored the no-copy/no-stack shape, then replaced every read-only
`__constref` parameter with an explicit `in` parameter:

```hlsl
[ForceInline]
void innerBwd_0(inout DiffPair_BigVec_0 input_0, in BigVec_0 dResult_0)

[ForceInline]
void s_bwdProp_inner_0(
    in s_bwdCallableCtx_inner_0 ctx_0,
    out BigVec_0 d_input_0,
    in BigVec_0 d_out_0)

[ForceInline]
void s_bwdProp_outer_0(
    in s_bwdCallableCtx_outer_0 _S3,
    out BigVec_0 _S4,
    in BigVec_0 _S5)

[ForceInline]
void _S10(inout DiffPair_BigVec_0 _S11, in BigVec_0 _S12)
```

The result stayed:

```text
memcpy=0
alloca=0
```

The generated Metal still has only `computeMain`; no `innerBwd`,
`s_bwdProp_inner`, `s_bwdProp_outer`, `_S10`, `dzero`, or `dadd` helper
definitions survive as separate functions. So for this repro, once the AD chain
is fully inlined and the aggregate context initializer is avoided, `in` versus
`__constref` does not affect the final optimized Metal LLVM.

This does not mean `in` and `__constref` are equivalent when inlining fails. The
earlier experiments suggest that avoiding non-inlined aggregate call boundaries is
the decisive factor.

## Compiler Implications

The experiment suggests two practical compiler-side changes:

1. Mark synthesized AD machinery as inline-critical when it wraps aggregate
   differential state.
   - This includes `apply_bwd`-style wrappers, synthesized backward propagation
     functions, and synthesized differential helpers such as `dzero`/`dadd`.
   - Without inlining `dadd`, stack materialization returns even with `__constref`.

2. Avoid emitting aggregate initializer helpers for simple context construction.
   - Prefer default construction plus direct field assignment when building
     generated AD callable contexts.
   - Alternatively, add an IR cleanup that rewrites this aggregate-constructor
     pattern before Metal emission.

`__constref` is still worth considering for read-only aggregate parameters, but
the ablation says it is not the primary fix for this repro. It is a robustness
improvement, not the reason the final `alloca` disappears.

## Compiler WAR Experiment

We implemented a compiler-side workaround with three pieces:

1. Synthesized `dzero`/`dadd` witness functions get `ForceInline`.
   - AST-synthesized `IDifferentiable` method witnesses receive a
     `ForceInlineAttribute`.
   - IR-synthesized differential-pair `dadd`/`dzero` helpers receive
     `IRForceInlineDecoration`.

2. Synthesized reverse-mode helper functions inherit force-inline from a
   force-inlined primal function.
   - For normal synthesized backward derivatives, `s_apply_*`, `s_bwdProp_*`,
     and `s_remat_*` are force-inlined when the primal function has
     `IRForceInlineDecoration`.
   - For legacy/custom backward derivatives converted to the new apply/remat/prop
     shape, the generated `s_bwdProp_*` wrapper is force-inlined when the primal
     function is force-inlined.

3. The final legacy backward wrapper also becomes force-inlined when it calls a
   force-inlined apply/propagate pair.
   - This covers the `_S10`-style wrapper emitted for `bwd_diff(outer)(...)`.

The repro source was annotated with:

```slang
[BackwardDerivative(innerBwd)]
[Differentiable]
[ForceInline]
BigVec inner(BigVec input) { ... }

[ForceInline]
void innerBwd(inout DifferentialPair<BigVec> input, BigVec.Differential dResult) { ... }

[Differentiable]
[ForceInline]
BigVec outer(BigVec input) { ... }
```

With the patched compiler, compiling this source to HLSL emits only
`computeMain`; no `innerBwd`, `s_bwdProp_*`, `_S10`, `dzero`, or `dadd` helper
definitions survive as separate functions. Compiling to Metal and then optimized
Metal LLVM gives:

```text
memcpy=0
512-byte memcpy=0
alloca=0
```

As a negative control, compiling the same repro with the three source
`[ForceInline]` annotations removed still emits helper functions in Metal:

```text
innerBwd_0
s_bwdProp_inner_0
s_bwdProp_outer_0
```

and optimized Metal LLVM reports:

```text
memcpy=5
512-byte memcpy=5
alloca=6
```

So this WAR is conditional on the user marking the primal/custom derivative path
as force-inline. It is not currently a blanket inline-all-AD-machinery change.

## What This Does Not Prove Yet

- It does not prove that forcing all AD machinery to inline is always acceptable
  for code size.
- It does not prove the same counts for every target backend; this study is
  specifically Metal plus Apple's optimizer.
- It does not prove that a general aggregate cleanup pass is easier than changing
  AD synthesis. It only shows that this one aggregate initializer is visible to
  Metal as a stack allocation unless we avoid it.
- It does not prove that `__constref` is unnecessary in general. It was not
  necessary for the final result here because the relevant calls were inlined.

## Current Working State

`issue-11085-default-forceinline.hlsl` currently represents variant D:

```text
ForceInline on all AD helpers
default/by-value read-only aggregate params
field assignment for _S14
```

Its observed result is:

```text
memcpy=0
alloca=0
```

To recover variant E in that file, add `__constref` to the read-only aggregate
parameters:

```hlsl
__constref BigVec_0 ...
__constref s_bwdCallableCtx_inner_0 ...
__constref s_bwdCallableCtx_outer_0 ...
```

Variant E is the HLSL form with:

```text
memcpy=0
alloca=0
```
