# Slice 156: Canonical UInt64 word reconstruction

## 1. Motivation

Consider an AnyValue conformer with one 64-bit field:

```slang
struct DoubleImpl : IValue
{
    double val;
}
```

AnyValue marshalling stores that field as two consecutive `UInt32` words. Its canonical
unmarshalling path loads those words and retains this final IR:

```text
%low   : UInt = load(...field0...)
%high  : UInt = load(...field1...)
%bits  : UInt64 = makeUInt64(%low, %high)
%value : Double = bitCast(%bits)
```

Direct NVVM already supported every scalar operation needed to reconstruct the bits, but it
rejected `makeUInt64` before provider discovery. The exact shape blocked three frozen-v1 AnyValue
layout workloads and one discovery workload, making it a four-workload canonical root cause across
both corpora.

## 2. Proposed solution

Recognize only `makeUInt64(UInt32 low, UInt32 high) -> UInt64`. Describe its lowering as a finite
compiler-owned recipe using the existing typed provider interface: zero-extend each word to
UInt64, construct a typed UInt64 constant 32, shift the high word, and bitwise-or it with the low
word.

Collect all three operation descriptors during preflight, validate both operands through the
ordinary SSA availability path, and bind the combined provider value directly to the canonical IR
instruction. Keep AnyValue's representation and provider ABI revision 30 unchanged.

## 3. Change summary

- `slang-emit-nvvm.cpp` adds one exact word-construction descriptor shared by requirement
  collection, value validation, and emission.
- Four repository shaders gain permanent direct-NVVM O0/O3 differential lanes.
- Frozen-v1 and discovery census snapshots are refreshed separately.
- The representative measurement manifest adds the newly unlocked AnyValue workload, bringing the
  exploratory SM70/80/90 set to thirteen gates.
- The design, capability ledger, completed plan, and this report record the representation and
  validation evidence.

## 4. Concepts and vocabulary

- **Word reconstruction**: combining ordered low and high 32-bit words into their exact 64-bit bit
  pattern without interpreting that pattern as signed or floating point.
- **Typed recipe**: a compiler-owned finite graph of generic provider operations whose complete
  descriptors are capability-checked before emission.
- **AnyValue unmarshalling**: reconstructing a concrete value from the fixed 32-bit-word payload
  used by lowered dynamic dispatch.
- **Healthy denominator**: workloads with a stable native CUDA/NVRTC reference; frozen v1 and
  discovery remain separate.

## 5. Process report

The input-shape audit starts at `slang-ir-any-value-marshalling.cpp`. Its 64-bit leaf path loads two
`UInt` payload fields and calls `IRBuilder::emitMakeUInt64(lowBits, highBits)`. That builder always
creates a `UInt64` result with the operands ordered low then high. A final linked-IR dump of
`layout-64bit-scalar` shows the same instruction separately reconstructing Double, Int64, and
UInt64 fields. The shape is canonical and intentionally retained, so the direct backend owns its
legalization; changing the producer or teaching downstream code to infer word order would be less
principled.

`_resolveNVVMUInt64WordConstruction` accepts only opcode `kIROp_MakeUInt64`, two operands, exact
scalar UInt64 result, and two exact scalar UInt32 operands. It initializes one UInt32-to-UInt64
conversion descriptor, one homogeneous UInt64 shift-left descriptor, and one homogeneous UInt64
bitwise-or descriptor. Signed inputs, vectors, alternate result types, or extra operands fail the
same deterministic preflight boundary and never mutate the provider module.

Both words are zero-extended before combination. This matters when bit 31 is set: a sign extension
would corrupt the opposite half of the final value. The high word is shifted by a provider-typed
UInt64 constant 32, and the low word is then ORed with it. The bit ranges are disjoint, so the
result is exactly `(UInt64(high) << 32) | UInt64(low)`. This is the same recipe used by the ordinary
LLVM emitter and the established direct-NVVM Double-from-words scalar helper, which independently
proves the provider operation family.

The requirements pass records every recipe step before builder discovery. Function validation
uses `_validateSelectedValue` for the low and high SSA producers and adds the result only after
both succeed. Emission retrieves those already-lowered words, applies the queried recipe, and maps
the result directly. No provider callback, feature flag, compatibility fallback, operand-graph
walk, custom type equivalence, fixture-name check, source reconstruction, or upstream layout patch
was added.

All four first blockers become correct against native CUDA at direct O0 and O3. Frozen
`layout-64bit-scalar`, `layout-64bit-vector`, and `layout-mixed-bitwidths` cover scalar, vector, and
mixed-width AnyValue layouts. Discovery `anyvalue-bulk-copy` covers a larger dynamic-dispatch
combination and becomes a representative measurement gate. The four files pass all 20 of their
direct and existing directives.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references. It moves from 381/385/381
to 384/388/384 O0/O3/both correctness, with zero old-correct loss. Across all rows, native remains
449 correct and three infrastructure; direct O0 is 397 correct, 42 preflight, eight runtime
mismatch, and five provider; direct O3 is 402 correct, 42 preflight, and eight runtime mismatch.

Discovery remains exactly 82 workloads/72 healthy references and moves from 58/58/58 to 59/59/59,
also with zero old-correct loss. Each direct mode has 59 correct, 13 preflight, two provider, seven
infrastructure, and one runtime-mismatch result. Exact ID comparisons show zero composition change
in either corpus. The selected NVVM prefix passes 427/427.

All thirteen representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90. The
new AnyValue gate measures 356.5 ms and 1240-byte PTX at direct O3 SM70 versus 482.8 ms and
9850-byte PTX through NVRTC O3. Direct O0 measures 346.0 ms and emits 182309-byte PTX. These remain
exploratory measurements, and the O0 size remains an optimization-quality signal rather than a
correctness failure.

The self-review inventory contains one exact resolver, one requirements closure, and one emitter
recipe. Each survives because removing it restores the same four canonical first blockers, and
the producer/type/operation contract proves this layer owns it. The repository formatter script is
run during final validation; unavailable tool dependencies, if unchanged on this machine, are
recorded in the completed plan. `git diff --check` and artifact integrity are required before the
commit.

Frozen both-mode correctness is now 384/427 (89.9%), satisfying the previously named
approximately-90% checkpoint. This slice does not declare corpus v2. The next bounded planning
step should propose a deduplicated composition and rationale while preserving both existing
denominators until explicit approval.
