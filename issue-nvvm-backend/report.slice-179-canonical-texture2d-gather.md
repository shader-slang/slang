# Slice 179: Canonical ordinary Texture2D gather

## Motivation

The frozen `tests/hlsl-intrinsic/texture-2d-gather.slang` workload stopped at canonical CUDA
`GenericAsm` for `Texture2D<float4>.GatherRed`. Gather is a common compute resource operation, and
the adjacent element-type fixture demonstrates that the same producer family spans scalar,
two-lane, and three-lane declared texture elements, Float32/Int32/UInt32 results, component
selection, and an offset overload. Handling that complete family unlocks a reusable resource
invariant rather than one fixture spelling.

## Proposed solution

Add one typed `GATHER` texture operation with an explicit component selector in provider ABI
revision 34. Compiler preflight recognizes the exact finalized ordinary CUDA-prelude helpers,
proves their complete texture/result/sampler/coordinate/offset contract, and passes only the
texture and coordinate runtime values to the provider. The provider emits exact PTX `tld4` for the
result kind and component. Keep comparison, array, cube, status, multiple-offset, and non-32-bit
gathers outside this bounded operation.

## Change summary

- Provider ABI revision 34 adds `SLANG_NVVM_TEXTURE_OP_GATHER` and descriptor `component` metadata;
  the host wrapper, real provider, and fake provider share its two-operand contract.
- Read-only texture description admits legal three-lane 32-bit elements, while each operation
  retains its own exact supported result shapes.
- Direct preflight and emission lower the base and single-offset ordinary Texture2D gather helpers.
- The LLVM provider emits float, signed, and unsigned four-lane gathers for components red through
  alpha and reconstructs their vector results.
- Focused fake/provider tests prove topology, supported/rejected descriptors, serialized IR, and
  operation-family reuse. Runtime and element-type fixtures gain permanent direct coverage.
- Frozen/discovery tables, Pareto artifacts, measurement manifest, plan, design documentation, and
  this report retain the complete evidence.

## Concepts and vocabulary

**Ordinary gather** means the non-shadow texture operation that returns four neighboring values for
one selected component. It excludes comparison/depth gathers.

**Declared texture element** is the `T` in `Texture2D<T>`. Its lane count need not match gather's
result: `Texture2D<float3>.GatherGreen` still returns `float4`.

**Component selector** is provider metadata 0 through 3 for red, green, blue, or alpha. It is not a
runtime LLVM value.

## Process report

Consider this example:

```slang
Texture2D<float3> texture;
SamplerState sampler;
float4 value = texture.GatherGreen(sampler, float2(0.5, 0.5));
```

The generated extension in `source/slang/hlsl.meta.slang` returns
`vector<T.Element,4>` and lowers this call to the exact assembly
`tex2Dgather<$TR>($0, ($2).x, ($2).y, 1)`. `$TR` deliberately denotes the four-lane result rather
than the three-lane declared element. `StmtLoweringVisitor::visitIntrinsicAsmStmt` produces the
one-block `IRGenericAsm` helper consumed by `_validateNVVMFunction`.

`_resolveNVVMTextureGatherGenericAsm` owns this shape because it is already finalized CUDA-prelude
IR. It accepts only component spellings 0 through 3 and proves a non-shadow, non-array Texture2D;
an ordinary sampler; a Float32 `float2` coordinate; a four-lane result whose scalar matches the
declared texture scalar; and either no fourth parameter or an exact signed `int2` offset. The
offset overload is intentionally valid: its producer explicitly states that CUDA `tex2Dgather`
does not support offsets and emits the same assembly after ignoring that parameter. The resolver
does not infer syntax or discard arbitrary operands.

`getNVVMSupportedReadOnlyTextureType` previously rejected a declared three-lane element before an
operation could classify it. That shape is canonical in the element-type fixture and resource
handles lower opaquely, so the resource description now admits lanes one through four. This does
not silently widen sample or fetch: `_isTextureOperationSupported` in both providers continues to
enforce each operation's exact lane set, and gather requires a four-lane result descriptor.

Preflight records `GATHER`, shape 2D, result type, and component. Emission skips the already-proven
sampler and optional offset, lowers the texture and coordinate, and calls the provider with two
operands. `NVVMIRBuilder::emitTextureOperation` was updated with that same operation-ID and operand
contract; without this change it returned `SLANG_E_INVALID_ARG` before dispatch, proving the
shielded wrapper is a required shared validation boundary rather than compatibility scaffolding.

The LLVM provider maps the descriptor to
`tld4.{r|g|b|a}.2d.v4.{f32|s32|u32}.f32`. The final `.f32` describes coordinate type: omitting it
serialized valid LLVM inline assembly but failed CUDA 12.9 `ptxas`, so the assembler gate proves
the provider owns the complete PTX spelling. LLVM inline assembly returns four scalar outputs;
the provider constructs the semantic four-lane vector with existing LLVM operations. The new
provider callback is justified because generic value operations cannot express sampled-resource
access.

The self-review inventory contains the exact resolver, resource lane widening, descriptor field,
host-wrapper case, provider branch, emission branch, and fake-result classification. All survive:
removing the resolver restores the measured GenericAsm preflight stop; removing lane three rejects
the canonical `Texture2D<float3>` fixture; removing the wrapper case prevents callback dispatch;
removing the provider or fake branches fails their focused tests. Non-gather descriptors require a
zero component, keeping the new metadata exact. No code checks fixture names, rebuilds source
syntax, walks arbitrary IR, weakens a diagnostic, adds a fallback, or repairs malformed upstream
IR.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references and advances from
416/416/416 to 417/417/417 O0/O3/both. `texture-2d-gather.slang#cuda-1` is the only changed row;
there are no old-correct regressions. All-row direct totals are 431 correct, three runtime
mismatches, and 18 preflight failures per mode. The obsolete `generic-asm-texture` cluster is gone.

Discovery remains exactly 82 workloads/72 healthy references at 72/72/72, with no changed row.
Its direct classification totals remain 72 correct, seven infrastructure failures, one runtime
mismatch, and two preflight failures per mode. The selected prefix passes 437/437 and the permanent
`nvvm` category passes 90/90.

CUDA 12.9 assembles the gather workload's native reference, direct O0 SM70, and direct O3
SM70/SM80/SM90 PTX. One-repetition measurements are 710.0 ms/8,945 PTX bytes for the reference
(which emitted SM75), 428.0 ms/6,519 bytes for direct O0 SM70, and 409.9 ms/1,091 bytes for direct
O3 SM70. Direct O3 stays 1,091 bytes at SM80 and SM90. These timings remain exploratory.
