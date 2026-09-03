# Slice 193: Preserve atomic-reduction semantics and support Half2 add

## Motivation

Consider the two remaining frozen-v1 workloads with the same first unsupported helper:

```slang
__atomic_reduce_add(half2AddTarget[0], half2(0.125h, 0.25h));
__atomic_reduce_add(reinterpret<RWStructuredBuffer<half2>>(*inputBuffer)[0], half2(1.0h, 2.0h));
```

Both native CUDA references were correct. Direct O0 and O3 stopped at:

```text
GenericAsm assembly=__slang_atomic_reduce_add($0, $1, (int)$2),
signature=Void(RefParam<vector<half,2>>, vector<half,2>, int)
```

The direct emitter already classified nine atomic-reduction spellings and lowered their scalar
forms through the typed atomic provider. Widening that text table for Half2 would have fixed the
fixtures while retaining target-string identity in a path whose standard-module producers are
known.

## Proposed solution

Give every ordinary reduction CUDA branch an operation-specific producer semantic and replace the
GenericAsm reduction classifier with one exact `IRNVVMIntrinsic` classifier. The semantic owns
add/subtract/min/max/bitwise/increment/decrement behavior; the finalized helper signature owns the
selected value type and whether a value parameter exists. Calls must still provide a canonical
global reference and literal relaxed order.

Admit only global relaxed Float16x2 add in the shared typed atomic catalog. The existing descriptor
already says floating point, 16 bits, two lanes, and the existing callback already accepts typed
pointer/value operands and returns the old value. The LLVM 14 provider can therefore emit the
packed CUDA atomic without changing ABI revision 35.

## Change summary

- Added nine internal atomic-reduction semantic IDs and source names.
- Tagged matching CUDA branches in both `core.meta.slang` and `hlsl.meta.slang`.
- Replaced atomic-reduction assembly matching in preflight, call validation, and emission with one
  semantic classifier and removed the obsolete GenericAsm path.
- Widened the shared atomic catalog, fake provider, and LLVM 14 provider for exact global relaxed
  Float16x2 add; no provider interface or ABI revision changed.
- Added real-provider query/emission coverage and expanded compiler fake-boundary coverage to
  observe the Half2 descriptor.
- Promoted both unlocked frozen workloads with permanent O0/O3 direct comparisons.
- Regenerated separate frozen/discovery census and Pareto artifacts and representative metrics.

## Concepts and vocabulary

**Atomic reduction** performs an atomic update without exposing the old value to Slang source.
**Producer semantic** is the internal operation identity attached where a standard module selects
target assembly. **Physical helper reference** is the generic helper-ABI pointer whose calls have
already been proven to originate in global storage. **Half2** is a two-lane Float16 vector carried
as one 32-bit register by the CUDA packed atomic instruction.

## Process report

`StmtLoweringVisitor::visitIntrinsicAsmStmt` stores each new semantic on the selected GenericAsm
producer. `_legalizeNVVMSemanticIntrinsics` replaces that terminator with
`IRNVVMIntrinsic(semantic-id)` and deliberately drops the CUDA string. Both producer families use
the same IDs: `hlsl.meta.slang::__atomic_reduce_*` supplies the free functions used by the two
motivating tests, while `core.meta.slang::Atomic<T>.reduce*` supplies the method family already
covered by `atomic-reduce-methods.slang` and `atomic-reduce-methods-float.slang`.

The exact shape reaching `_resolveNVVMTaggedAtomicReduction` is a complete one-block helper with one
semantic terminator and a void result. Add/subtract/min/max/bitwise helpers have `(ref T, T, int)`;
increment/decrement have `(ref T, int)`. This is canonical and intentionally allowed because those
are the finalized standard-module definitions. The resolver checks the complete body and signature,
unwraps `Atomic<T>` only through the established atomic-type helper, and obtains kind/width/lanes
from the selected `T`. It does not inspect source names or reconstruct the discarded assembly.

Removing any producer tag now leaves an unsupported GenericAsm and fails the existing method tests
or compiler unit. Removing the Half2 catalog widening preserves the semantic helper but makes both
motivating frozen rows fail preflight. This establishes separate evidence for operation identity
and the new type shape.

Calls are audited independently. `_validateNVVMFunction` recognizes a reduction callee by its tagged
terminator, then `_isNVVMGlobalHelperReferenceArgument` proves the first argument is a canonical
global-storage producer and `_asExecutableI32Constant` proves relaxed order. The retained negative
case with a local reference still stops at `atomic reduction global reference`; no target-string or
fixture fallback was added.

`NVVMSemantics::isSupported` is the single catalog consulted by compiler preflight, fake provider,
facade queries, and the real provider. Its only new shape is add + global + relaxed + Float16x2.
Half3, Half4, shared Half2, vector min/max/bitwise, and BFloat16 remain rejected. The descriptor and
callback layouts do not change, so advancing the forward-only ABI would add no expressive power.

In `_emitAtomicOperation`, the provider verifies an LLVM `<2 x half>` pointee and value against that
descriptor. It bitcasts both to the 32-bit pointer/register types required by PTX, emits
`atom.global.add.noftz.f16x2`, and restores the unused old value to `<2 x half>`. These bitcasts are
the instruction's physical register contract, not an alternate semantic type. The ordinary scalar
atomic implementation and all other shapes remain unchanged.

The special-case inventory contains four entries. The semantic catalog survives because two named
standard-module producers and existing method/free-function tests prove ownership. The Float16x2
catalog widening survives because exactly two frozen workloads require it and negative provider
queries bound adjacent shapes. The LLVM packed boundary survives because LLVM/NVVM has no ordinary
vector atomic-RMW representation for this CUDA operation, and focused serialization/runtime/PTX
tests prove it. The fake-provider vector validation survives because it mirrors the same descriptor
contract and is exercised by the compiler test. No helper, fallback, syntax reconstruction, or
arbitrary operand walk was added.

Frozen v1 retains exactly 452 identities and 427 healthy MVP references. Direct O0, O3, and both
advance from 421 to 423, with exactly the two motivating gains and zero old-correct regressions.
Across all 452 rows, each direct mode records 437 correct, 14 preflight failures, and one
infrastructure failure. The former generic-asm-atomic cluster is eliminated. The four remaining
healthy gaps are three distinct helper-ABI/substandard types and one `RequirePrelude` marker.

Discovery retains exactly 82 identities and 72 healthy references at 72/72/72. Its all-row totals
remain 72 correct, seven infrastructure failures, one runtime mismatch, and two preflight failures
in each direct mode. No discovery workload was newly unlocked, which is expected because none uses
the frozen Half2 producer shape.

The focused Half2 module produces 6,206 bytes of O0 PTX and 1,300 bytes of O3 PTX. Both contain
`atom.global.add.noftz.f16x2`; O3 assembles to 3,432-byte SM70/SM80 cubins and a 3,872-byte SM90
cubin. The three representative application gates also assemble in every measured configuration.
For the resource/aggregate/helper gate, median compile time was 355.5 ms native, 246.8 ms direct O0
SM70, and 258.3 ms direct O3 SM70, with PTX sizes 8,889, 6,102, and 919 bytes. These remain
exploratory measurements rather than controlled benchmarks.

Release compiler/test and isolated LLVM 14 provider builds passed. Focused provider/compiler tests,
both motivating native/direct runtime files, and both existing `Atomic<T>.reduce*` files passed.
The selected NVVM unit prefix passes 439/439 and the permanent NVVM category passes 102/102.
