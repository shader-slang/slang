# Tiled-brass material: initial complex-corpus assessment

## Motivation

The supplied isolated MaterialX tiled-brass shader combines 7,110 source lines of layered material
logic, generic interfaces, aggregate/array transport, bindless texture sampling, lookup buffers,
control flow, and floating-point math. Its two 64-thread compute entries exercise different outputs:
`eval_buffer` computes a BSDF value and PDF; `sample_buffer` computes a direction, PDF, weight and
flags. This provides a concrete application workload for a future backend that compiles correctly,
compiles faster than the NVRTC path, and produces better executable code.

The current request is bounded: preserve the workload and assess the existing compiler, then discuss
next slices while keeping other feature work moving. No feature loop or compiler fix is started.

## Proposed solution

Add a separate compile-only complex corpus with both named entry points, preserve the supplied
shader, and assess NVRTC O3 versus direct NVVM O0/O3 under the same target contract. Keep the existing
frozen 452-identity and discovery 82-identity runtime corpora unchanged. Those corpora own runnable
inputs and expected outputs, which this attachment does not yet supply.

The user confirmed that `__TARGET_CUDA__=1` is the intended application option and approved placing
it in the shader. The corpus copy adds that define and one explanatory comment and normalizes CRLF
to LF; all other source text and the license header are preserved. Original and adapted SHA-256
identities are recorded in `complex-corpus.manifest.json`.

## Change summary

- `tests/cuda/complex/tiled_brass_material.slang`: intact application workload with its two entries.
- `tests/cuda/complex/README.md`: corpus scope, reproducible command, and missing runtime contract.
- `issue-nvvm-backend/complex-corpus.manifest.json`: entry identities, target SM80, source hashes,
  provenance, feature tags, and explicit compile-only status.
- `issue-nvvm-backend/run-complex-corpus.py`: records every requested outcome, including rejection;
  reuses the existing toolkit runner's command execution, timeout, hashing, and library selection.
  It validates source hashes, fresh PTX target/entry declarations, and assembly before success.
- `issue-nvvm-backend/assessment.tiled-brass.json`: durable initial measurements and diagnostics.
- The assessment plan/report and design-document link preserve this handoff without changing any
  production compiler, provider, ABI, frozen/discovery manifest, or slice-201 status.

## Concepts and vocabulary

A **compile-only cell** is one source/entry/backend/optimization combination. It passes only after
fresh PTX and successful assembly, and says nothing about runtime correctness. A **descriptor handle**
is the typed Slang representation of a bindless resource; a CUDA texture ultimately carries a 64-bit
texture object. **Preflight** validates linked Slang IR before the provider emits LLVM/NVVM. Slang
phase timers are nested; adding them together double-counts compilation work.

## Process report

### The source branch matters before comparing backends

Without the application define, the original attachment takes its non-CUDA `uint2` descriptor path.
NVRTC rejects conversion of that vector into `CUtexObject`/`SamplerState`, while direct NVVM reports
`CastUInt2ToDescriptorHandle`. Those failures are a configuration mismatch, not a fair NVVM baseline.
After selecting the confirmed CUDA branch, both NVRTC entries compile successfully.

Consider the relevant part of `TextureHandle.sample` in the retained workload:

```slang
public __generic<LodSampler : ILodSampler>
    T sample(LodSampler lod_sampler, float2 uv, const T default_value = T(T.Element(0)))
{
    if (!is_valid())
        return default_value;
    This resolved_handle = resolve_udim(uv);
    if (!resolved_handle.is_valid())
        return default_value;
    if (FLIP_V_COORDINATE)
        uv.y = 1.0 - uv.y;
#ifdef __TARGET_CUDA__
    Texture2D<T> texture = DescriptorHandle<Texture2D<T>>(uint64_t(resolved_handle.texture_index));
    SamplerState sampler;
#else
    Texture2D<T> texture = DescriptorHandle<Texture2D<T>>(uint2(resolved_handle.texture_index, 0));
    SamplerState sampler = DescriptorHandle<SamplerState>(uint2(resolved_handle.sampler_index, 0));
#endif
    return lod_sampler.sample(texture, sampler, uv);
}
```

`resolve_udim` similarly constructs a `Texture2D<uint>` descriptor for its indirection lookup.
The standard-library `DescriptorHandle<T>.__init(uint64_t)` in `source/slang/hlsl.meta.slang` is
explicitly available for CUDA and has the intrinsic opcode `kIROp_CastUInt64ToDescriptorHandle`.
The IR dump retains that canonical opcode with a `UInt64` operand and typed texture descriptor result.
The resource constructor then uses `getDescriptorFromHandle` to obtain the texture.

This is valid intentional input, not an accidental alternative representation to repair in the
shader. `legalizeIRForNVVM` leaves the conversion intact; `_validateNVVMFunction` in
`slang-emit-nvvm.cpp` does not admit the integer-to-descriptor opcode and reports E52017 before
provider emission. Its existing `_getNVVMDescriptorHandleConversion` admits only resource/descriptor
identity conversions. `asNVVMSupportedDescriptorHandleType` and `NVVMTypeLoweringContext::lowerType`
already preserve selected descriptors using their underlying resource representation. The ordinary
C-like emitter treats descriptor conversions as operand forwarding, allowing NVRTC to compile them.

This identifies a focused backend support boundary to investigate next. It does **not** establish
that adding this conversion alone will compile the complete shader: preflight stops at the first
unsupported shape. A future fix must audit the selected texture representation and avoid treating
all resource families, including pointer/count buffer descriptors, as interchangeable integer handles.
No such fix or speculative workaround is included here.

### Measured baseline

Compiler checkout: `74f227e10`, native Linux **Debug**, GCC 13.3, isolated LLVM14 provider,
CUDA 12.9.2 (NVCC/NVRTC 12.9.86), target **SM80**. Each successful cell uses one warmup and three
sequential timed CLI invocations. No other build or benchmark ran during the samples. Failed cells
stop on their first attempt and receive no successful compile-time metric.

| Entry           | NVRTC O3 compile + assembly | NVVM O0                               | NVVM O3        |
| --------------- | --------------------------- | ------------------------------------- | -------------- |
| `eval_buffer`   | Pass                        | E52017 `CastUInt64ToDescriptorHandle` | Same rejection |
| `sample_buffer` | Pass                        | E52017 `CastUInt64ToDescriptorHandle` | Same rejection |

| NVRTC O3 observation           | `eval_buffer` | `sample_buffer` |
| ------------------------------ | ------------: | --------------: |
| Median process wall time       |       2.569 s |         2.619 s |
| Median `compileInner`          |   2,202.33 ms |     2,243.51 ms |
| Median `frontEndExecute`       |   1,340.27 ms |     1,339.16 ms |
| PTX bytes                      |        72,360 |         103,321 |
| Cubin bytes                    |        70,944 |          92,088 |
| Registers reported by ptxas    |            48 |              63 |
| Stack-frame bytes              |           592 |             624 |
| Spill-store / spill-load bytes |         0 / 0 |           0 / 0 |

These are exploratory Debug-compiler timings, not production compilation-speed targets. Front-end
cost is substantial in this build, so backend-only timing cannot establish an end-to-end win. The
source emits 18 E31227 warnings about treating `constexpr` variables as `const`; they remain visible
in logs and no source changes were made to suppress them. Register/stack/spill and artifact-size
observations are not evidence that one backend produces faster kernels. No kernels were executed.

### Corpus/tool validation

The real material run contains exactly six cells: two successful NVRTC compile/assembly results and
four explicit direct preflight rejections. Its exit code is 1, correctly preserving incomplete
support. An already-supported `nvvm-core-execution.slang` control passed all three modes and
assembly with exit 0. A corrupted source-hash manifest exited 2 before invoking compilers; zero
samples were rejected. Source reconstruction verified that removing the agreed define/comment and
normalizing newlines reproduces the supplied attachment. Python syntax and repository whitespace/
formatting checks cover the new tooling and metadata; no compiler rebuild is necessary.
Discover-only checks preserved the exact 452 frozen identities and all 82 discovery identities
with zero frozen-source overlap.

Raw commands, timing samples, phase timers, compiler/provider/toolkit hashes, PTX, cubins and
assembler reports are under `build/nvvm-tiled-brass/baseline/`. The original-option probes and a full
IR dump remain under `build/nvvm-tiled-brass/`. The durable summary is `assessment.tiled-brass.json`.
The new helpers only validate corpus inputs and measure compiler outcomes; none manipulate semantic
IR, admit an unsupported operation, or change historical classifications.

### Choices for discussion before the next loop

1. Complete the pending slice-201 wave/quad acceptance as its own bounded task, preserving progress
   on existing feature work.
2. Consider a narrow CUDA integer-to-texture-descriptor slice, with focused positive/negative tests
   and these two entries as progress probes. Reassess the next actual blocker afterward rather than
   promising that all material support follows from one opcode.
3. Define representative material, texture, LUT and output-comparison fixtures before claiming
   execution correctness. In particular, confirm how the shader's packed texture index maps to a
   valid CUDA texture object; successful compilation does not validate the host binding ABI.
4. Once both paths compile and execute the same workload, measure optimized compiler builds under
   identical flags. Separate process/session overhead, Slang phases, downstream compilation, and
   kernel runtime. Then compare generated code using correctness, registers/spills, and actual GPU
   timing rather than PTX size alone.

A reasonable cadence is one reusable feature slice followed by a material-corpus checkpoint, with
broader feature/regression work retained in between. This report proposes that discussion; it does
not select or start the next development loop.
