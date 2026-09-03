# Slice 188: Audit and stage frozen runtime mismatches

## Motivation

Frozen corpus v1 reported three direct-NVVM runtime mismatches in both O0 and O3. Reading the logs
showed that this headline combined unlike failures. `bound-check-zero-index.slang` executed and
returned wrong values. `byte-address-buffer-atomic-mixed-width-12265.slang` stopped because a
profile-upgrade warning differed from the subprocess expectation. `anyvalue-layout.slang` never
compiled: `lowerBitCast` raised `Unable to generate bit_cast code for the given type` while
processing an opaque descriptor handle.

Treating all three as runtime failures hid the next actionable producer and made the Pareto report
misleading.

## Proposed solution

Classify failures by the first stage that actually fails. Preserve canonical descriptor-handle
bit transport until direct NVVM preflight can either select its target-owned physical recipe or
reject the exact adjacent shape. Make census classification recognize capability-profile and CUDA
renderer setup failures before treating expected/actual harness text as runtime output. Keep the
one genuine bounds divergence open after proving that its control option is absent from direct IR.

## Change summary

- Direct bit-cast lowering now preserves exact unsigned `uint2` and `uint4` descriptor transport.
- `anyvalue-layout` receives a permanent direct-NVVM diagnostic lane for its unsupported `uint2`
  descriptor shape.
- Census classification treats profile upgrades, unavailable CUDA support, and renderer creation
  failures as infrastructure even when the harness returns zero for an ignored test.
- Frozen Pareto ownership distinguishes descriptor transport and capability-profile setup.
- Complete three-mode frozen and discovery artifacts record the corrected taxonomy.

## Concepts and vocabulary

**Runtime mismatch** means both compilation and launch succeeded but the deterministic result
buffer differs. **Preflight failure** is a compiler-owned rejection before provider mutation.
**Descriptor transport** is the canonical bit cast between opaque `DescriptorHandle<T>` and the
unsigned word vector produced by source `reinterpret` or AnyValue marshalling.

## Process report

Consider the two source expressions
`reinterpret<uint2>(DescriptorHandle<Texture2D<float4>>)` and the existing AnyValue
`DescriptorHandle<StructuredBuffer<float>> <-> uint4` transport. Their producer intentionally
keeps the descriptor semantic type opaque; only the target knows its physical resource value.
`BitCastLoweringContext::processBitCast` previously preserved only the four-word form. On the
two-word form, `readObject` reached the opaque descriptor as a leaf and raised an internal error.

The new `shouldPreserveDirectNVVMDescriptorBitCast` accepts exactly one descriptor side and one
unsigned 32-bit vector side of two or four lanes, only for direct NVVM. The four-lane raw-buffer
shape continues to `_getNVVMRawBufferDescriptorBitCast` and its established physical recipe. The
two-lane `anyvalue-layout` shape reaches `_validateNVVMFunction`, which emits E52017 for
`bitCast type: vector<uint,2> -> DescriptorHandle`. This layer owns the deferral because common
lowering cannot inspect target representation, while direct preflight owns exact support. Removing
the helper reproduces the internal error; the new diagnostic test proves that unsupported adjacent
shape remains rejected. No arbitrary vector, signed payload, resource spelling, or fallback was
admitted.

The mixed-width atomic log contains no result buffer. The selected first CUDA directive defaults
to SM70 while the kernel requires SM90, so Slang emits E41012 and upgrades the profile. Both native
and direct reference setup are unhealthy for that row. `_classify_result` now recognizes this
toolchain/setup outcome before the harness's generic expected/actual wrapper. The Pareto producer
is capability selection and profile validation, not compiled PTX execution.

The bounds row is the only real runtime divergence. Its directive passes
`SLANG_ENABLE_BOUND_ZERO_INDEX` to Slang's generated CUDA path, where `slang-cuda-prelude.h`
selects `SLANG_BOUND_ZERO_INDEX`. Direct NVVM consumes linked IR and never emits or preprocesses
that CUDA text. Two direct O3 compilations with and without the define produced byte-identical
2,163-byte PTX with SHA-256
`3AD64F789B2C04D775199393D708764E005FDEC74C3B873276718B67FAF6FC70`. Native PTX changed from
11,322 to 12,717 bytes and its hashes differed. There is therefore no canonical bound-fix
operation for the emitter to implement. A future fix must introduce a producer-side target option
and IR representation; this slice does not infer bounds semantics from a fixture or downstream
macro.

The self-review inventory contains one direct-only shape predicate, one classifier ordering
change, two infrastructure markers, two Pareto ownership cases, and one diagnostic directive. All
survive the revert drill described above. The descriptor predicate handles a canonical,
producer-owned target boundary; the classification cases report observed stages rather than
weakening a compiler diagnostic. No provider callback, compatibility path, syntax reconstruction,
fixture-name check, or emitter special case was added.

The Release build succeeds. The selected NVVM prefix passes 437/437, the permanent category passes
92/92, and the focused AnyValue file passes 3/3 with one unrelated DX12 lane ignored. Frozen corpus
v1 remains 452 workloads and 427 healthy MVP references at 418/418/418 O0/O3/both with zero
old-correct regression. All-row direct results are now 432 correct, one runtime mismatch, one
infrastructure failure, and 18 preflight failures per mode. Discovery remains 82 workloads and 72
healthy references at 72/72/72; its direct classifications remain 72 correct, seven
infrastructure failures, one runtime mismatch, and two preflight failures per mode. Provider ABI
revision 34 is unchanged.
