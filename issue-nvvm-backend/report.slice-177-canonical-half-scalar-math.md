# Slice 177: Canonical Half scalar math

## Motivation

The frozen `hlsl-intrinsic/scalar-half.slang` workload stopped at the first canonical helper
`$P_min` specialized as `half(half, half)`. The same workload deliberately combines ordinary math
with NaN, infinity, signed-zero, fused arithmetic, and out-parameter decomposition, so supporting
only minimum would have repeated the same legalization boundary one operation at a time.

## Proposed solution

Use one exact compiler-side recipe for homogeneous scalar-Half CUDA helpers: convert each Half
operand to Float32, invoke the existing typed Float32 semantic, and narrow a floating result once.
Keep `sign`, `frexp`, and `modf` result topologies explicit. Add provider operations only for the
canonical Float32/64 semantics revision 32 could not express: `sinh`, `cosh`, `tanh`, fused `fma`,
and the two pure `modf` projections.

## Change summary

- One shared exact GenericAsm spelling table now serves direct typed operations and legalization.
- The scalar recipe resolver admits the complete homogeneous Half math family and explicit Half
  `frexp`/`modf` pairs.
- Provider ABI revision 33 adds six generic semantic IDs, libdevice mappings, and a private `modf`
  projection implementation.
- Fake-provider topology and real-provider serialization tests cover promotion, ternary FMA, and
  decomposition.
- `scalar-half.slang` gains permanent direct O0/O3 differential lanes.
- Frozen/discovery evidence, representative measurements, plan, design, and capability ledger are
  updated without changing either corpus denominator.

## Concepts and vocabulary

**Promoted Half semantic** means CUDA evaluates the library operation in Float32 after exact Half
input conversion, then converts a floating result back to Half.

**Projection operation** exposes one result of a two-result library function through the existing
single-result typed provider callback. The provider's temporary pointer is not part of its ABI.

## Process report

Consider the source chain in `scalar-half.slang`: it calls `min`, `sinh`, `fma`, `frexp`, and
`modf` on scalar Half values. CUDA selection in `hlsl.meta.slang` chooses exact intrinsic-assembly
spellings. `StmtLoweringVisitor::visitIntrinsicAsmStmt` creates `IRGenericAsm`; specialization and
linking leave it as the sole executable instruction in a one-block helper with a complete typed
signature. This is canonical producer output, not malformed IR.

The diagnostic progression exposed `$P_min half(half,half)`, `$P_sinh half(half)`, `$P_fma
half(half,half,half)`, `$P_frexp half(half,OutParam<int>)`, then `$P_modf
half(half,OutParam<half>)`. `_findNVVMGenericAsmOperationSpelling` now centralizes only exact final
spellings. `_resolveNVVMScalarIntrinsicRecipe` additionally proves every ordinary parameter is a
non-out scalar Half and the result is Half, or signed i32 for `sign`. It then constructs conversion,
typed operation, and optional narrowing steps. Adjacent mixed, vector, or different-result shapes
remain unsupported and retain deterministic diagnostics.

`frexp` stores a signed-i32 exponent, while `modf` stores a Half integral part, so neither is forced
through the homogeneous value template. Their explicit recipes promote once, query both Float32
projections during preflight, narrow only floating outputs, and use the existing typed store. The
resolver remains the common source for capability collection and emission.

Revision 32 already expressed Half/Float conversion and most Float32 operations. It did not express
hyperbolic math, a fused ternary operation, or `modf`. Revision 33 adds those exact typed semantics.
The provider calls `__nv_sinh[f]`, `__nv_cosh[f]`, `__nv_tanh[f]`, and `__nv_fma[f]`; using an LLVM
FMA intrinsic failed legacy NVVM serialization, while multiply-plus-add would not preserve fusion.
For `modf`, `_emitModfProjectionOperation` allocates provider-local integral storage, calls
`__nv_modf[f]`, and returns the requested fraction or integral value. That pointer is an LLVM
implementation detail, so no pointer callback or compiler-side ABI spelling is introduced.

The self-review inventory contains the shared exact spelling lookup, the promoted recipe, the two
explicit decomposition recipes, and the provider-local `modf` projection. Each survives: removing
them restores one of the measured canonical stops or changes FMA/decomposition semantics. The
fake recorder capacity increase is test infrastructure required by the bounded combined source.
No code checks a fixture name, parses a substring, reconstructs syntax, walks arbitrary operand
graphs, weakens diagnostics, adds a fallback, or patches malformed upstream IR.

Frozen corpus v1 stays exactly 452 workloads/427 healthy references and advances from
414/414/414 to 415/415/415 O0/O3/both. `hlsl-intrinsic/scalar-half.slang#cuda-1` is the only gain;
there are no old-correct regressions. All-row direct totals are 429 correct, three runtime
mismatches, and 20 preflight failures per mode. Discovery stays exactly 82 workloads/72 healthy
references at 72/72/72 with no changed row. The selected prefix passes 434/434 and the permanent
`nvvm` category passes 84/84.

The representative gate assembles through CUDA 12.9 for native NVRTC, direct O0 SM70, and direct
O3 SM70/SM80/SM90. At SM70, standalone one-repetition measurements are 589.8 ms and 64,486 PTX
bytes native, 329.8 ms and 126,480 bytes direct O0, and 382.8 ms and 33,284 bytes direct O3. These
measurements remain exploratory.
