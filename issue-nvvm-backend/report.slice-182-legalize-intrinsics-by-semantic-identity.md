# Slice 182: Carry fixed NVVM intrinsic semantics from their producers

## Motivation

The fixed direct-NVVM value catalog still paired each typed provider operation with a CUDA
`__intrinsic_asm` string. Preflight and emission independently recovered semantics from that text,
so adding an overload meant maintaining a source spelling, a typed catalog row, and two consumers.

## Proposed solution

Attach a small compiler-internal semantic tag at each standard-module producer. Preserve it while
lowering intrinsic assembly, then have the NVVM legalization stage replace the tagged
`IRGenericAsm` terminator with `IRNVVMIntrinsic(semantic-id)`. Resolve the overload from that
identity and the complete specialized helper signature, never from CUDA text.

## Change summary

- Added optional semantic tags to intrinsic-assembly statements and 105 fixed producer cases.
- Added an internal semantic decoration and typed NVVM intrinsic terminator.
- Rewrote tagged helpers at the NVVM-ready legalization boundary.
- Removed CUDA spellings and the fallback text/signature matcher from all 72 fixed catalog rows.
- Retained exact text recognizers for richer recipe/resource/atomic families pending typed payloads.

## Concepts and vocabulary

**Fixed value catalog** means the finite table mapping a semantic value-operation ID plus complete
scalar/vector signature to one provider operation. **Producer tag** is compiler-internal metadata
on a standard-module `__intrinsic_asm` statement. It is consumed before NVVM preflight and is not a
source-language compatibility mechanism. **Richer family** means an operation whose semantics
need data beyond a value-operation ID, such as texture shape/component or atomic memory roles.

## Process report

Consider the CUDA standard-module implementation of scalar `sin`:

```slang
__intrinsic_asm(nvvmSin) "...";
```

Parsing records `nvvmSin` separately from the CUDA assembly body. AST-to-IR lowering emits the
ordinary `IRGenericAsm` used by the CUDA route and decorates it with the typed value-operation ID.
After target specialization has selected a concrete helper signature, `legalizeIRForNVVM` consumes
the decoration and replaces the terminator with `IRNVVMIntrinsic`. CUDA compilation still owns the
text; direct NVVM does not carry it past legalization.

Preflight proves the canonical one-block helper, reads the operation ID, and derives exact result
and operand types from the specialized function signature. Emission uses the same typed provider
descriptor. Homogeneous Half helpers remain a compiler-owned Float32 promotion recipe selected by
the semantic ID and exact signature. Void barriers explicitly emit a void return after their value
operation. An unknown internal tag is out of contract and asserts at its construction boundary.

The migration covers 72 catalog rows through 105 standard-module cases: ordinary scalar and
libdevice operations, conversions and bit operations, fixed wave operations, execution-register
queries, and barriers. The catalog's `genericAsm` field, semantic type matcher, and catalog text
lookup are gone.

The inventory also tested whether this representation could replace every remaining GenericAsm.
It cannot. A texture gather needs texture shape, result lanes, component, sampler/offset topology;
surface operations need access and coordinates; atomics need storage/value roles; compound wave
recipes need intermediate data flow. Encoding those as opaque extra operands would recreate a
generic target escape instruction. Those valid canonical families keep their existing exact
recognizers until a later producer-side slice gives each an adequate typed representation.

The self-review inventory contains the parser tag, semantic-name table, decoration, legalization
rewrite, typed terminator, fixed-operation resolver, Half recipe, and void-return path. Each exists
for the producer-to-consumer trace above. No fixture-name check, syntax reconstruction, fallback,
provider callback, or newly admitted IR shape was added. Provider ABI revision 34 is unchanged.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references at 418/418/418 O0/O3/both,
with no old-correct regression. All-row direct classifications remain 432 correct, three runtime
mismatches, and 17 preflight failures per mode. Discovery remains exactly 82 workloads/72 healthy
references at 72/72/72. The selected prefix passes 437/437 and the permanent NVVM category passes
92/92.
