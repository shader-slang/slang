# NVVM feature and evidence matrix

This matrix describes qualified combinations, not a claim of complete Slang or CUDA support. The
[architecture](nvvm-backend.md) owns representations and invariants. [STATUS](../../issue-nvvm-backend/STATUS.md)
owns the current accepted checkpoint and loaded compiler identity; its referenced manifests own exact
inventories and outcomes. Source test links below identify durable contracts; the accepted evidence
records which compiler and inputs were actually tested. Deeper historical evidence is available through the
[archive guide](../../issue-nvvm-backend/HISTORY.md).

Helper parameter/result admission and provider lowering use the same `NVVMTypeInfo` role policy.
Argument provenance, exported ABI restrictions and physical layout compatibility remain separate
requirements; sharing type classification does not broaden any qualified role.

## Reading the evidence

- **GPU** means an independently expected output was qualified on the recorded platform. It does not
  mean every type/role combination, host layout or optimization has been tested.
- **Compile/assembly** means compilation and fresh cubin assembly, without a runtime conclusion.
- **Unit/IR** means provider/emitter/representation checks. Fake-provider success is not GPU execution.
- **Boundary** means a deliberate unsupported or failing case whose outcome remains visible.
- **Focused research** means a bounded experiment; its sources may be in archived evidence rather than
  the permanent corpus. It does not silently expand the implementation or main corpus.

The maintained runtime comparison is NVRTC O3, NVVM O0 and NVVM O3. NVRTC O3 is a comparison, not a
universal oracle. Explicit expected values, complete buffers, guards and execution counts establish
correctness. An unsupported compile has no GPU result. A deliberate corruption rejected by an oracle
is successful test infrastructure evidence, not a passing shader.

Current broad qualification is on native Ubuntu 24.04, L4 SM89, driver 580.126.09, target SM80,
CUDA 12.9.2/NVRTC 12.9.86 and LLVM 14. Other historical toolkits/devices retain their own provenance.
The installed compiler can be older than Git HEAD; source revision alone does not identify loaded code.

## Compute, values and memory

| Region                            | Qualified domain / evidence                                                                                                           | Boundaries and durable anchors                                                                                                                                                                                                                                                                                                                                                                                                           |
| --------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Direct PTX routing                | Opt-in direct route, target-scoped selector, explicit architecture; `case nvvm` refines CUDA for direct PTX                           | NVRTC remains default. [Compiler units](../../tools/slang-unit-test/unit-test-nvvm-compiler.cpp)                                                                                                                                                                                                                                                                                                                                         |
| Compute launch and builtins       | Conventional and raw compute entry contracts; explicit LLVM register reads for thread/block/grid dimensions; group barrier; GPU       | No graphics/OptiX entry claim. [Core execution](../../tests/cuda/nvvm-core-execution.slang), [ordinary entry](../../tests/cuda/nvvm-ordinary-compute-entry.slang)                                                                                                                                                                                                                                                                        |
| Scalar numeric values             | Selected signed/unsigned 8/16/32/64-bit integers, Bool and IEEE Half/Float/Double; typed transport/conversion and admitted operations | Exact operation descriptors govern overloads; type admission alone grants no operation. [Mixed numeric](../../tests/cuda/nvvm-mixed-numeric.slang), [Half](../../tests/cuda/nvvm-half-values.slang), [Double](../../tests/cuda/nvvm-float64-values.slang)                                                                                                                                                                                |
| Vectors                           | Selected widths 2–4; construction, extraction, swizzle updates, typed selection and operation families                                | Boolean lane references need local SSA normalization; arbitrary pointer escape remains rejected. [Typed select](../../tests/cuda/nvvm-typed-select.slang), [Boolean lanes](../../tests/cuda/nvvm-local-boolean-lanes.slang), [negative](../../tests/cuda/nvvm-boolean-lane-reference-unsupported.slang)                                                                                                                                  |
| Matrices                          | Selected logical numeric matrices lowered to aggregate values and physical storage; GPU and layout checks                             | Physical row/column layout and host packing remain explicit. [Float matrices](../../tests/cuda/nvvm-float-matrix-values.slang), column-major qualification below                                                                                                                                                                                                                                                                         |
| Helpers and control flow          | Typed calls/results, phi values, loops/switches, finite copyable records/arrays, noinline functions, selected mutable references      | Internal by-value ABI and exported CUDA ABI are separate. [Helper values](../../tests/cuda/nvvm-helper-copyable-values.slang), [mutable forwarding](../../tests/cuda/nvvm-mutable-parameter-forwarding.slang)                                                                                                                                                                                                                            |
| Pointer-bearing helper values     | Canonical device/UserPointer and qualified recursive helper transport                                                                 | Access/address-space/layout operands and producer provenance are checked. [Pointer forwarding](../../tests/cuda/nvvm-mutable-pointer-payload-forwarding.slang)                                                                                                                                                                                                                                                                           |
| Explicit-layout pointer transport | Scalar/C Device record pointers, signed32 offsets and UInt64 address observation; allocated-address O0/O3 checks                      | Strides follow Slang layout rules: motivating record48/40. CUDA C++ uses40 for both; preserve the discrepancy. Entry roots and checked offsets only; no dereference, helpers, stored/reconstructed pointers or Std430 admission. [Positive](../../tests/cuda/nvvm-layout-pointer-transport.slang), [negative](../../tests/cuda/nvvm-layout-pointer-transport-unsupported.slang).                                                         |
| Parameter groups and resources    | Selected conventional globals, uniforms, constant/parameter blocks, structured and byte-address buffers, resource-bearing aggregates  | Separate launch, parameter-group and structured-buffer representation; no universal aggregate ABI. [Multiple resources](../../tests/cuda/nvvm-conventional-global-multi-resource.slang), [compact storage](../../tests/cuda/nvvm-compact-vector-storage.slang)                                                                                                                                                                           |
| Borrowed float3 storage           | Readonly borrowed fields/array elements preserve native storage across mutable use; GPU and provider-memory regression                | Readonly access must not imply compact parameter-group storage. [Borrowed vector storage](../../tests/cuda/nvvm-borrowed-vector-storage.slang)                                                                                                                                                                                                                                                                                           |
| Shared/local memory and atomics   | Selected finite shared storage, typed integer atomic families and admitted memory orders; unit/integration/corpus gates               | Every element type, pointer role and atomic overload still needs admission. [Emitter units](../../tools/slang-unit-test/unit-test-nvvm-emitter.cpp), [integration units](../../tools/slang-unit-test/unit-test-nvvm-integration.cpp)                                                                                                                                                                                                     |
| Thread-local context              | Selected per-invocation global/context values and finite copyable aggregates pass ordinary single-dispatch checks                     | Repeated dispatch of bare `static State state;` in the copyable-context fixture fails in all three modes: both routes leave its private aggregate uninitialized. The implicit-initialization language contract needs resolution. Does not imply arbitrary global initialization or host ABI. [Context](../../tests/cuda/nvvm-thread-local-global-context.slang), [copyable context](../../tests/cuda/nvvm-copyable-kernel-context.slang) |
| Receiver snapshots                | Canonical aggregate parameter snapshots preserve values across resource mutation; GPU and material compile evidence                   | Aggregate-memory ordering issue is fixed; material runtime is separately scoped below. [Snapshot](../../tests/cuda/nvvm-aggregate-param-snapshot.slang), [resource snapshot](../../tests/cuda/nvvm-aggregate-param-resource-snapshot.slang)                                                                                                                                                                                              |
| Nested integer aggregate stores   | Nested records and root/wrapped/multidimensional arrays; NVVM O0/O3 GPU and 39 provider shape/alignment checks                        | Qualified libNVVM store workaround; focused NVRTC optimized copies still fail. [Root](../../tests/cuda/nvvm-nested-array-root.slang), [wrapper](../../tests/cuda/nvvm-nested-array-wrapped.slang), [multidimensional](../../tests/cuda/nvvm-nested-array-multidimensional.slang)                                                                                                                                                         |

Scoped coherent pointer memory admits naturally aligned Int/UInt32/64 for exactly Device/global
and Workgroup/shared. The canonical memory attributes select scoped relaxed GPU/CTA operations;
coherent reads do not become read-modify-write operations or invariant loads. Existing permissions
and proven pointer roots remain required. The [guarded fixture](../../tests/cuda/nvvm-coherent-pointer-memory.slang)
uses disjoint writes, a group barrier, mirrored reads, ordinary-memory controls and both-end guards.
The [negative fixture](../../tests/cuda/nvvm-coherent-pointer-memory-unsupported.slang) keeps CUDA C++,
SM60, mismatched scopes/spaces, float/vector values, weak alignment and integer-derived roots closed.
The NVVM arm requires SM70+, with focused qualification at SM80; the Vulkan capability arm is
unchanged. Other scopes, new pointer roles and volatile semantics are not admitted. The original
physical-pointer and redundant-load corpus programs contain races and are not runtime oracles.
See [the scoped validation procedure](../../issue-nvvm-backend/RESULTS.md#scoped-coherent-pointer-memory).

Core execution helpers and varying-parameter legalization use named
`llvm.nvvm.read.ptx.sreg.*` intrinsics for all twelve thread/block/grid coordinates.
The named-intrinsic boundary admits zero-argument scalar i32 register reads, the three void
barrier/fence operations described below, and scalar integer `llvm.ctpop`, `llvm.bitreverse`,
`llvm.ctlz` and `llvm.cttz`, scalar Float32/Float64 `llvm.sqrt`, and zero-operand
`llvm.nvvm.read.ptx.sreg.clock`/`clock64` signatures. It does not admit arbitrary LLVM
assembly. ABI 46 carries explicit ordinary `__intrinsic_asm` operands; pure provider queries validate LLVM registry signatures and
immediate-constant requirements before module creation. Actual emission separately validates value
handles and rejects conflicting symbols without mutation. CUDA source and NVRTC retain CUDA target
selection, including when an explicit `nvvm` capability is supplied.
[Target/argument fixture](../../tests/cuda/nvvm-target-switch-explicit-args.slang) and
[varying composition](../../tests/cuda/nvvm-execution-register-varyings.slang) retain their contracts.

Core integer count, reverse, high-bit and low-bit APIs use named scalar primitives plus ordinary
Slang width/sign/zero handling. Live independent bit-loop oracles cover signed/unsigned scalar operations and
vectors of lengths 2/3/4 at widths [8](../../tests/cuda/nvvm-integer-intrinsics-8.slang),
[16](../../tests/cuda/nvvm-integer-intrinsics-16.slang),
[32](../../tests/cuda/nvvm-integer-intrinsics-32.slang) and
[64](../../tests/cuda/nvvm-integer-intrinsics-64.slang), with literals, completion and guard checks.
All widths pass NVVM O0/O3; widths 32/64 also pass NVRTC O3. NVRTC 8/16 scan/reverse prelude helpers
remain missing, and their original compile failures remain recorded. The separate
[small-count fixture](../../tests/cuda/nvvm-integer-count-small.slang) passes all three modes.
Public signed 8-bit population count preserves uint32 sign extension (32 for -1, 25 for -128); direct
i8 population count independently checks the same-width results (8 and 1). The
[producer check](../../tests/cuda/nvvm-integer-producers.slang) requires four named intrinsics and
explicit false scan flags. Scalar registry admission does not imply vector LLVM intrinsic support.

The [target-switch helper test](../../tests/language-feature/capability/target-switch-dead-helper.slang)
checks that discarded NVVM helper branches do not diagnose during CUDA linking, while a live
unavailable switch still rejects. [Layout controls](../../tests/language-feature/capability/target-switch-layout-lifetime.slang)
cover uniform and resource-only CUDA/HLSL/SPIR-V programs; later HLSL/SPIR-V pruning still removes
unused resources. Semantic module version 43 rejects older modules before AST/IR decoding; metadata inspection
and source fallback remain available. Retired integer IDs 46/47 are not accepted through a compatibility
shim. Numeric count45/low48 are also retired after their compound wave consumers moved to core.

Public `round` uses named `__nv_roundf`/`__nv_round` calls validated against actual definitions in
an immutable snapshot of the selected libdevice. The same bytes reach libNVVM. Half uses canonical
FloatCast widening/narrowing around Float32 round, preserving NVVM ties-away and CUDA Half ties-even.
Numeric operation64 and the old tag/text recognizers are retired. Scalar library ABI admission
does not admit vector calls; the public vector/matrix bodies map to scalar operations.
The [Float32](../../tests/cuda/nvvm-round-32.slang), [Float64](../../tests/cuda/nvvm-round-64.slang)
and [Half](../../tests/cuda/nvvm-round-half.slang) contracts cover live scalar/noinline helper,
vector2/3/4 and matrix2x2 results with independent integer/rational oracles, signed-zero checks,
NaN classification, guards and completion. Half compares both tie policies unconditionally.
The [oracle checker](../../extras/test-generators/check-nvvm-round-oracles.py) recomputes frozen
expectations without a compiler; the [removed-tag control](../../tests/cuda/nvvm-round-removed-tag.slang)
checks the source boundary. Current acceptance and compiler identity remain owned by STATUS.

Public `ceil`, `floor` and `trunc` use direct scalar Half `llvm.ceil`/`llvm.floor`/`llvm.trunc`
with signatures checked by the LLVM registry. Checked Half trunc lowers to the exact pure PTX
`cvt.rzi.f16.f16` primitive with bit-preserving i16 assembly transport because libNVVM 12.9
rejects its LLVM declaration and Half assembly operand types; ceil/floor keep their
named declarations. Their six Float32/64 names still come from selected
libdevice definitions; IDs 42/54/57 and their tag/text paths are
retired. The combined
[Float32](../../tests/cuda/nvvm-directed-rounding-32.slang),
[Float64](../../tests/cuda/nvvm-directed-rounding-64.slang) and
[Half](../../tests/cuda/nvvm-directed-rounding-half.slang) fixtures check 64 live IEEE inputs each,
with signed zero, subnormal and integer/precision neighbors, infinity and NaN class. Each input
has 45 scalar/helper/vector/matrix observations, distinct operation/shape masks, guards and completion.
The [integer-bit oracle checker](../../extras/test-generators/check-nvvm-directed-rounding-oracles.py)
validates the frozen rationally generated constants without executing a compiler.

Public `frac` uses an explicit NVVM `x - floor(x)` core body; `fract` forwards to it. Named selected
floor calls, ordinary subtraction and canonical casts replace numeric 59, its tag and text path.
Half evaluates the whole expression in Float32 before narrowing once. Every finite Half residual
is exactly Float32, proving finite RN-even equivalence with CUDA's direct Half expression. Tiny
negative inputs can round to one; finite integers and either signed zero return positive zero.
The [Half](../../tests/cuda/nvvm-frac-half.slang), [Float32](../../tests/cuda/nvvm-frac-32.slang) and
[Float64](../../tests/cuda/nvvm-frac-64.slang) fixtures have 64 live IEEE inputs per width and 26
observations per lane: scalar/noinline/vector2/3/4/matrix2x2 `frac`, plus scalar/noinline/vector2/3/4
`fract`. There is no matrix `fract` overload. Eleven shape-error bits, both scalar raw results,
per-lane completion and endpoint guards occupy 386 words per fixture. Finite results compare exact
bits; infinity and NaN inputs require NaN classification. All three modes use one mathematical
oracle, with no target-policy marker. The [standalone integer checker](../../extras/test-generators/check-nvvm-frac-oracles.py)
recomputes the independent rational expectations and validates the fixture/output contract without
compiler execution or ignored manifests. The [removed-tag test](../../tests/cuda/nvvm-frac-removed-tag.slang)
checks the frontend boundary. Real-provider tests retain the named-floor/ordered-subtract contract
in both serializers and reject reserved 59 without mutation; emitter tests verify the public
composition and Half cast ordering. Current acceptance and exact compiler identity are owned by STATUS.

Public `rsqrt` uses selected named Float32/64 library functions and canonical Half widening,
Float32 evaluation and one narrowing. Numeric 65, its tag and text route are retired. Real-provider
tests derive signatures from selected definitions, including deliberate name/width crossings,
preserve typed calls in both serializers and reject invalid insertion without mutation. Emitter
tests verify direct Half widen/call/narrow edges and live scalar/vector/matrix outputs.
The [Half](../../tests/cuda/nvvm-rsqrt-half.slang), [Float32](../../tests/cuda/nvvm-rsqrt-32.slang)
and [Float64](../../tests/cuda/nvvm-rsqrt-64.slang) fixtures contain 64 unique IEEE inputs and 15
observations per lane. Exact rational references define a test union of reference-spacing radius
and encoding-step neighborhoods (Float32 N = 2, Float64 N = 1), with empirical, non-guaranteed library
scope. Separate NVVM and CUDA Half policies use the exact narrowed images of the library union
and the PTX epsilon 2^-22.9 set, respectively. These policies agree on the selected Half inputs;
a shared selector drives both the admission table and visible policy marker.
Six shape-error bits, raw scalar low/high pairs, completion and guards occupy 259 Half or 258
other words. The [standalone checker](../../extras/test-generators/check-nvvm-rsqrt-oracles.py)
recomputes tables without a compiler or ignored manifest and exercises six negative certificate
controls. The [removed-tag test](../../tests/cuda/nvvm-rsqrt-removed-tag.slang) checks the frontend
boundary. Preserve all nine pre-migration buffers byte-for-byte, including raw NaNs; payloads remain
unpromised by numerical admission. Current acceptance and exact compiler identity are owned by STATUS.

Public `exp` selects named `__nv_expf`/`__nv_exp` through the same selected-definition boundary.
Half preserves Float32 library evaluation before one narrowing. Numeric 55, its tag and text route
are retired; real-provider tests retain both serializers and reject reserved operation 55 without mutation.
Emitter tests preserve preflight-before-output and direct Half widen/call/narrow edges.
The [Half](../../tests/cuda/nvvm-exp-half.slang), [Float32](../../tests/cuda/nvvm-exp-32.slang)
and [Float64](../../tests/cuda/nvvm-exp-64.slang) fixtures contain 80 / 62 / 62 unique inputs and 15
scalar/noinline/vector2/3/4/matrix2x2 observations per lane. Six shape-error bits, raw scalar pairs,
completion and guards occupy 323 / 250 / 250 words. Half markers `55040` (NVVM) and `1209` (CUDA) identify
policies, not the current module version; the same target selector chooses marker and admission.

The default library union uses reference-spacing radius or encoding-step distance, N = 2 for Float32
and N = 1 for Float64, as an empirical non-guaranteed test convention. Zero uses minsubnormal spacing;
maxfinite uses the conceptual next binade; infinity uses only ordered encoding neighbors. Exact
special-input classifications override the union. NVVM Half is its exact Float32-to-Half image;
CUDA Half retains encoded-FMA, ex2 input/output FTZ, Half narrowing and four correction FMAs.
All four correction inputs distinguish candidate sets; baseline outputs at `0x1f79` / `0x25cf` differ
intentionally. These bounded policies imply neither universal accuracy nor target equivalence.

The [standalone checker](../../extras/test-generators/check-nvvm-exp-oracles.py) independently
reconstructs the corpus, references, sets, stage images and fixture contracts using integers,
without compiler execution or ignored manifests. Its 38 synthetic controls cover midpoint parity,
endpoint rules, FTZ, constants and corrections. An optional ignored proposal audit checks generator
proof fields and rejects 16 mutations. The [removed-tag test](../../tests/cuda/nvvm-exp-removed-tag.slang)
checks the frontend boundary. Preserve all nine raw baseline buffers, including NaNs. Current
acceptance and exact compiler identity remain owned by STATUS.

Public `exp2` selects named `__nv_exp2f`/`__nv_exp2`; Half preserves the selected Float32 call
before narrowing once. Numeric 56, its tag and legacy text are retired. Direct real-provider
rejection/no-mutation and emitter preflight tests remain independent of the module-version gate.
The [Half](../../tests/cuda/nvvm-exp2-half.slang), [Float32](../../tests/cuda/nvvm-exp2-32.slang)
and [Float64](../../tests/cuda/nvvm-exp2-64.slang) fixtures contain 74 / 69 / 69 unique inputs,
15 live observations per lane and 299 / 278 / 278 output words. They preserve full scalar bits,
completion and guards; Half markers `56042` and `1209` identify NVVM and CUDA policies.

Library admission follows the same test-defined endpoint convention with independently certified
base-two references. CUDA Half uses ex2 input/output FTZ, Float32 FMA with multiplier `2^-24` and
then RN16; NVVM Half narrows its Float32 library result once. Candidate sets agree on this corpus,
while synthetic midpoint candidates distinguish the policies. The existing exponential checker
with `--operation exp2 --check --self-test` reconstructs 212 references and runs 57 controls:
38 retained shared checks and 19 exp2 checks. Optional preparation-proposal auditing rejects 14
mutations. Default invocation still checks exp unchanged. The [removed-tag test](../../tests/cuda/nvvm-exp2-removed-tag.slang)
checks fresh tagged source; current acceptance and exact tested identity remain owned by STATUS.

Public `log`, `log2` and `log10` use six selected Float32/Float64 library names and canonical Half
widening/evaluation/narrowing. Operations 60/61/62 and their tag/text paths are retired. The
[log](../../tests/cuda/nvvm-log-half.slang), [log2](../../tests/cuda/nvvm-log2-half.slang) and
[log10](../../tests/cuda/nvvm-log10-half.slang) families each have Half/Float32/Float64 fixtures:
91/62/62 inputs for log and log2, and 91/62/80 for log10. Each input has 15 scalar/helper/vector/matrix
observations, guarded full scalar bits and completion; all 27 mode buffers must preserve their
respective baseline bytes. Current acceptance remains owned by STATUS.

The [independent checker](../../extras/test-generators/check-nvvm-log-oracles.py) certifies signed
references using directed integer logarithm intervals. Ordinary library admission uses the
empirical test-defined spacing/encoding union reflected by sign, with Float32 radii 1/1/2 and
Float64 radius 1. NVVM Half narrows the Float32 set; CUDA Half checks ideal RN-even results on the
finite corpus. Bounded source/correction controls distinguish the installed Half paths without
claiming comprehensive PTX approximation qualification. Exact specials preserve signed input-zero
handling, positive output zero at one, infinities and NaN class; NaN payloads remain unpromised.

CUDA double-log10 checks RN32 input conversion, Float32 log10 admission and exact widening because
the existing `F64_log10(float)` wrapper narrows. This qualified preservation result is not evidence
of true-double accuracy. The corpus excludes negative tiny doubles that narrow to negative zero;
its common NaN classification does not establish that untested case. Direct retired-ID rejection
and legacy-text preflight tests remain independent of old-module version rejection.

Public `sin`, `cos`, `acos`, `asin`, `atan`, `atan2`, `pow`, `tan`, `sinh`, `cosh`, `tanh`, `fma`
and `fmod` use 26 selected Float32/Float64 library names. Shared table-driven units cover their
real-provider signatures, explicit argument order and scalar/vector public paths. Half `fma` uses
`llvm.fma` with one nearest-even rounding of the exact product and sum; the remaining operations
retain Half widen/call/narrow composition. The [Half FMA fixture](../../tests/cuda/nvvm-half-fma.slang)
checks independent exact finite bits, signed zero/Inf, NaN class, helper and vector paths, including
precise mode. This deliberately corrects the old Float32-intermediate double rounding.
[RESULTS](../../issue-nvvm-backend/RESULTS.md#native-half-fused-multiply-add-contract) records its
contract and graphics-backend limitations. Approximate Half exp2/tanh and round tie-policy changes
remain separate work. Twelve retired numeric IDs reject without module mutation; all thirteen legacy tag/text
routes and legacy `sincos` text reject before output. FMOD 58 remains for canonical `kIROp_FRem`.
Core `sincos` assigns sine then cosine, floating `mad` calls `fma`, and integer `mad` uses multiply/add.
The small [composition smoke](../../tests/cuda/nvvm-core-math-composition.slang) checks exact
runtime-loaded integer `mad` results across signed/unsigned widths, scalar Half/Float32/Float64
`sincos` at zero, aliased output order and floating `mad`, under NVVM O0/O3. Integer `mad` and scalar
Half `sincos` are new support. These focused checks establish routing and composition, without
claiming a new per-operation numerical qualification. Current acceptance remains owned by STATUS.

Public Half bit transport, packed `f16tof32`/`f32tof16`, double word conversion and
`isfinite`/`isinf`/`isnan` use core expressions. Numeric NaN operation 67 is reserved; seven old
transport spellings and nine width-specific classification shapes reject before provider mutation.
The small [core bits smoke](../../tests/cuda/nvvm-core-bits.slang) checks eight runtime-loaded rows
through CUDA and NVVM O0/O3: signed/unsigned Half bit transport including NaNs, ignored upper packed
bits, zero-extension after packing, asymmetric double word order, aliased out stores, and all three
floating widths' zero/subnormal/finite/infinity/NaN classifications. NaN numerical conversions check
classification only; raw bit transport checks exact bits. Existing Half value/narrowing and vector
classification fixtures cover the shared operations. Current focused acceptance remains in STATUS.

Public abs/min/max/sign now use core bit operations, comparisons and selected fabs/fmin/fmax
library calls. Half abs preserves raw NaN payloads, signed abs emits wrapping INT_MIN semantics, and sign
maps NaNs and both zeros to zero. Unsigned abs and signed integer sign are new support. The compact
[core values fixture](../../tests/cuda/nvvm-core-values.slang) checks runtime-loaded integer edges,
Half quiet/signaling NaN abs, zero and NaN sign, shared integer reductions and shared/global 64-bit
inc/dec wrap under NVVM O0/O3. Both cells now pass with the original wrapping expectations. The
previous O3 signed16 INT_MIN failure is resolved by provider normalization at semantic integer
consumers: comparison, division/remainder, right shift, widening and integer-to-float conversion.
An explicit signed/unsigned widen/narrow pair prevents PTX's excess high bits from changing those
observations. Shift counts normalize around width conversion; scalar and vector lanes retain their
original LLVM types. Half/BF16 transport is unchanged. The
[narrow integer fixture](../../tests/cuda/nvvm-narrow-integer-semantics.slang) passes O0/O3 and covers
overflow/underflow, signedness reinterpretation, helper/vector paths, mixed-width shifts, named bit
intrinsics and a wrapped-zero vector index. Historical failures and primitive experiments remain in
accepted evidence; this does not claim arbitrary raw pointer arithmetic qualification. Named provider
units cover all six library signatures; raw IDs 49/68 and 80/81 and retired source routes reject.
Numeric MIN/MAX is also retired after canonical wave consumers moved to core.

Both public atomic reduction APIs now produce canonical Atomic IR. Their existing Relaxed-only
memory-order and pointer admission is unchanged; HLSL inc/dec explicitly forwards order and rejects
floating inputs. Shared integer reductions and 64-bit inc/dec are newly exposed through that same
contract. The existing integer/float/Half reduction fixtures and Float32 ByteAddress add/UInt64 CAS
fixtures cover existing behavior. ByteAddress typed views require naturally aligned offsets.
Named clock signatures retain side-effecting PTX and the existing
[clock observations fixture](../../tests/cuda/nvvm-clock-observations.slang); they do not become
readnone LLVM calls. Current focused results and compiler identity remain owned by STATUS.

Public `frexp` and `modf` now call the four selected libdevice definitions with checked local
output pointers; Half promotes to Float32 and narrows floating outputs in core, retaining frexp's
integer exponent. Scalar Float32/64 modf
is newly exposed by this producer migration. Provider units cover all four exact signatures,
wrong pointee/role/address-space/attributes and rejection without mutation; compiler units keep
both result channels live, including Float64 modf. Existing
[Float32 frexp](../../tests/bugs/frexp.slang), [Float64 frexp](../../tests/bugs/frexp-double.slang)
and [Half scalar math](../../tests/hlsl-intrinsic/scalar-half.slang) own numerical runtime evidence.
No fresh Float64 modf numerical claim follows from provider/compiler checks. IDs 69/70/77/78 and
the pointer-output CUDA text recipes are retired; the LLVM registry still rejects output pointers.

Size/alignment use canonical typed queries with explicit CUDA layout. Unit checks preserve general
zero size and the CUDA unsized-array wrapper's size 16/alignment 8, while NVVM size helpers reject
zero and signed Int32 overflow. Field offset uses a typed source query with its exact field key
captured before optimization. NVVM retains the direct same-base restriction and CUDA layout;
CPP/CUDA restore the original call and bodies. Focused units cover equal-valued distinct fields,
generic record specialization, dead queries, wrong-base rejection and the removed text spelling.
The existing [array-layout](../../tests/cuda/cuda-array-layout.slang) and
[parameter-block alignment](../../tests/cuda/param-block-alignment.slang) fixtures cover both
NVVM and CUDA source paths. No active NVVM CUDA-text recognizer remains.

Public `sqrt` uses the named LLVM intrinsic with canonical Half promotion to Float32 and narrowing.
Numeric 36, its tag and text path are retired. Direct named Half/vector sqrt is outside provider
admission; public vector/matrix operations map to scalar calls. Pure queries reject floating
INTEGER_CONSTANT metadata, and typed emission preserves registry attributes and ownership checks.
Sqrt-only programs make no library query or load. The [Half](../../tests/cuda/nvvm-sqrt-half.slang),
[Float32](../../tests/cuda/nvvm-sqrt-32.slang) and [Float64](../../tests/cuda/nvvm-sqrt-64.slang)
fixtures cover 64 live IEEE inputs, 15 scalar/helper/vector/matrix observations per lane, signed
zeros, subnormal/normal boundaries, infinity, NaN classification, guards and completion. CUDA Half
uses an independently bounded approximate-root oracle; NVVM uses exact Float32 evaluation followed
by Half rounding. Distinct visible mode markers prevent accepting the wrong policy. All nine
observed output buffers retain their pre-migration bytes; NaN payloads remain unpromised.
The [standalone integer oracle](../../extras/test-generators/check-nvvm-sqrt-oracles.py) requires no
compiler or ignored manifest. The [removed-tag test](../../tests/cuda/nvvm-sqrt-removed-tag.slang)
checks the frontend diagnostic; preserved old modules separately qualify numeric-tag retirement.

Core bit reinterpretation and Half-value conversions use canonical NVVM `BitCast`/`FloatCast`
instructions. The [conversion API fixture](../../tests/cuda/nvvm-conversion-intrinsics.slang) checks
all six signed/unsigned/float reinterpretation directions against exact input bits, scalar and
heterogeneous vector2/3/4 Half conversions against independent encodings, literal rounding boundaries
and an untouched guard. Numeric NaNs require NaN class; reinterpretation preserves payload bits.
The [producer IR test](../../tests/cuda/nvvm-conversion-producers.slang) requires typed casts for
all six reinterpretation signatures and scalar/vector Half conversion paths. CUDA vector conversion
uses canonical casts and the existing emitter's per-lane conversion; CUDA's scalar intrinsics cannot
accept whole vectors. Packed uint Half conversions and Double word assembly remain separate contracts.

Ordinary Var/Load/Store decisions are now retained in the checked emission plan: allocation role,
alignment, conversion recipe, load flags and pointer ABI/provenance. BF2 identity and BF3/BF4 lane
conversion are planned before provider mutation. Native readonly borrows remain distinct from compact
storage and from immutable locations. This changes ownership, not the supported language surface.
Structured-buffer roots and direct loads also retain their checked view/access/operand facts.
Recursive Boolean/aggregate/vector3 conversions use recipes built during preflight, including
explicit-stride construction; children inherit checked parent address facts in dominance order.
The [recursive round trip](../../tests/cuda/nvvm-structured-bool-vector-roundtrip.slang) covers nested
Boolean fields and float3 storage in both directions. No transforming Slang IR storage pass is claimed.

Half-vector helper parameters/results use physical integer lane transport while body arithmetic and
storage retain their selected Half representations. [Parameter transport](../../tests/cuda/nvvm-half-vector-helper-parameters.slang)
and [result transport](../../tests/cuda/nvvm-half-vector-helper-results.slang) separately cover all
65,536 encodings per lane in two correlated input families, preserving earlier results across later
calls. This includes signed zeros and NaN payload bits without performing Half arithmetic; it is not
a Cartesian product of lane combinations. [Effectful composition](../../tests/cuda/nvvm-half-vector-helper-effectful.slang)
checks a Half-vector result beside an observed call effect. [Exports](../../tests/cuda/nvvm-half-vector-helper-exports.slang)
cover retained source boundaries; separate frozen-declaration PTX callers qualify the existing direct
export ABI, including semantic lane offsets. CUDA-prelude binary interoperability and return padding
contents are not qualified. [Direct static units](../../tools/slang-static-unit-test/unit-test-nvvm-type-lowering.cpp)
check exact classifier boundaries and canonical/physical/storage cache orders.

Unsupported bodies, signatures or roles fail preflight; successful lowering of an adjacent type is
not authorization to guess an ABI. Negative emitter tests also assert that rejection occurs before
provider mutation. The provider's serialization tests establish physical operation shape separately
from executable source contracts.

## BF16 and FP8: role-specific matrix

Here **record value** means internal construction/extraction/whole load/store/call/return/phi in the
qualified finite record domain. **Local reference** means the exact qualified mutable Generic root
and field provenance, not arbitrary pointers. Formats remain semantic identities even when LLVM uses
integer storage.

| Feature / role                                         | Current state and evidence                                                                                                                                                                         | Explicit limits / regression                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| ------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| BF16 scalar transport, literals, local mutable storage | Supported as physical i16; GPU includes all 65,536 bit encodings and signed literals                                                                                                               | Raw transport preserves NaN payloads; casts have separate policy. [Scalar](../../tests/cuda/nvvm-bf16-scalar.slang), [signed literals](../../tests/cuda/nvvm-bf16-signed-literals.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| BF16↔Float32                                           | Scalar and equal-width vector conversion; nearest-even narrowing and exact bit expansion on qualified SM80                                                                                         | NaN narrowing classification only; no integer, Half or Double cast admission. [Scalar](../../tests/cuda/nvvm-bf16-scalar.slang), [vector values](../../tests/cuda/nvvm-bf16-vector-values.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| BF2/BF3/BF4 register and internal helper values        | Supported `<N x i16>` construction/extraction and branch/phi transport; GPU                                                                                                                        | No external CUDA helper ABI claim or inferred explicit vector Select contract. [Vector values](../../tests/cuda/nvvm-bf16-vector-values.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| BF2/BF3/BF4 local mutable references                   | BF2 native-vector storage; BF3/BF4 component arrays; symmetric raw lane conversion; GPU with both cache orders                                                                                     | Does not grant component-pointer, device/resource, readonly or exported roles. [Local vectors](../../tests/cuda/nvvm-bf16-local-vectors.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| Flat local BF16 records including BF3/BF4              | Integer and BF16 scalar/vector fields; field load/store through proven local root; layout and exhaustive transport evidence                                                                        | BF3/BF4-containing whole record values, nesting, arrays and external storage not admitted. [Local records](../../tests/cuda/nvvm-bf16-local-records.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| CUDA BF16 layout queries                               | Canonical scalar/vector/matrix/wrapper layout matches selected CUDA forms; reflection, IR and GPU query tests                                                                                      | Layout metadata, including record-array queries, is not runtime memory admission. [Layout](../../tests/cuda/nvvm-bf16-cuda-layout.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| BF16 dot widths 2–4                                    | Core source-ordered product/sum composition using scalar BF16 FMA; GPU                                                                                                                             | No unrestricted FP32 accumulation, general BF16 arithmetic/comparisons or width-0/1 contract. [Dot](../../tests/cuda/nvvm-bf16-dot.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| FP8 scalar transport and finite literals               | E4M3/E5M2 i8 values, internal helpers, same-size bitcasts and same-format selection; GPU over all 256 encodings per format                                                                         | Transported nonfinite bytes supported; nonfinite literals rejected. [Transport](../../tests/cuda/nvvm-fp8-scalar-transport.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| FP8→Float32                                            | Both formats, exact finite values and E5M2 infinity; GPU over every byte                                                                                                                           | NaN payload/sign unspecified; no native FP8 hardware required on SM80. [Widening](../../tests/cuda/nvvm-fp8-widening.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| Shared finite FP8 folding                              | Repaired finite normal/subnormal conversion and nearest-even rounding; exhaustive independent unit evidence and source regression                                                                  | Shared overflow policy differs from CUDA SATFINITE. [Finite literals](../../tests/cuda/nvvm-fp8-finite-literals.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| Float32→FP8 runtime narrowing                          | Research only; no backend admission                                                                                                                                                                | Prototype nearest-even/saturation evidence does not qualify production conversion. Integer/Half/Double conversions, FP8 arithmetic and vectors remain excluded.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| Flat/nested mixed record values                        | Integer leaves with FP8/BF16 scalar/BF2, at least one substandard descendant; local mutable storage and internal value transport supported                                                         | No BF3/BF4 whole values or resource/shared/readonly/exported roles. [Flat](../../tests/cuda/nvvm-substandard-records.slang), [nested](../../tests/cuda/nvvm-nested-substandard-records.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| BF2 record-field component address                     | Supported from actual field of a qualified local record, preserving exact pointer/index checks                                                                                                     | Does not qualify BF3/BF4 component pointers or bare FP8 pointers. [Flat records](../../tests/cuda/nvvm-substandard-records.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Nested dynamic AnyValue payloads                       | Two runtime-selected conformers, Natural unpack/CUDA local/mutate/repack and saved interface copy; three-mode GPU                                                                                  | Raw bit transport; no new arithmetic or arbitrary interface ABI. [Substandard](../../tests/cuda/nvvm-nested-substandard-dynamic.slang), [integer control](../../tests/cuda/nvvm-nested-integer-dynamic.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| Local mixed record arrays                              | Nonempty fixed arrays of existing integer/FP8/BF16/BF2 identity records; dynamic selection, snapshots, internal value, out/inout and readonly parameters, source returns via OutParam; focused GPU | Address roots require local Var or first-block admitted parameters of internal helpers; calls prove origin and access before type equality. Readonly borrows retain native storage and ordinary loads. No native array/pointer results, exported references, wrapper fields, multidimensional arrays, BF3/BF4 elements or external storage. [Transport](../../tests/cuda/nvvm-local-substandard-record-arrays.slang), [snapshots](../../tests/cuda/nvvm-local-substandard-record-array-copies.slang), [value parameters](../../tests/cuda/nvvm-substandard-record-array-parameter.slang), [references](../../tests/cuda/nvvm-local-substandard-record-array-references.slang) |
| Local-record Generic pointer helper result             | Excluded                                                                                                                                                                                           | Exact synthetic Generic-result boundary is source-reviewed; public UserPointer negative tests are a different shape.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |

BF16 record-array feasibility experiments with raw LLVM, physical BF3/BF4 lane arrays and successful
CUDA metadata queries do not by themselves establish runtime record-array support. Similarly, all
65,536 scalar patterns in a two-element array do not establish a 65,536-element allocation or scalable
compile time. Nested dynamic tests cover correlated field combinations, not a Cartesian product.
[Direct static units](../../tools/slang-static-unit-test/unit-test-nvvm-local-record-arrays.cpp)
qualify each Value/Storage/HelperParameter first-use order for fixed arrays of flat and nested
identity records, including forbidden-role lookups before and after successful caching. Forged
pointer roots cannot acquire the local allocation's checked address permission. These tests do not
qualify arbitrary nested dynamic records or array reference/result/export signatures. The value
parameter fixture checks caller saved/live arrays, callee-local mutation and a returned selected
record with integer oracles. All 65,536 original bit patterns are covered, with four slot/lane
combinations occurring 16,384 times each; this is correlated coverage, not a Cartesian product.

The architecture preserves the numeric details needed to extend these regions: CUDA BF sizes and
alignments, separate Natural/CUDA packing, BF16 NaN/cast policy, source-ordered dot and FP8 overflow.
These details must accompany any widening of a role predicate.

## Waves, synchronization and observations

| Region                                            | Current contract / evidence                                                                  | Limits / regression                                                                                                                                                                                                                          |
| ------------------------------------------------- | -------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Lane reads, ballots, votes, rotation and prefixes | Named primitives and core payload, reduction/prefix, quad and rotation composition           | Exact type widths and mask participation govern legality. [Shuffle widths](../../tests/cuda/nvvm-wave-shuffle-widths.slang), [rotation](../../tests/cuda/nvvm-wave-rotation.slang)                                                           |
| Hardware active mask                              | Side-effecting convergent snapshot; implicit aggregate helpers compose the required ballot   | Distinct from logical active-mask synthesis; source branch membership need not imply reconvergence. [Hardware mask](../../tests/cuda/nvvm-hardware-active-mask.slang), [aggregate mask](../../tests/cuda/nvvm-implicit-aggregate-mask.slang) |
| Masked integer MIN/MAX                            | Narrow and 64-bit signed/unsigned identities in core prefix/reduction folds                  | Identity bits are width-specific; no invalid shift by 64. [Narrow](../../tests/cuda/nvvm-narrow-masked-minmax.slang), [64-bit](../../tests/cuda/nvvm-i64-masked-minmax.slang)                                                                |
| Masked Float/Double MIN/MAX                       | Source-ordered reduction and prefix rules, signed-zero/NaN/singleton behavior; GPU           | Ordinary min/max calls are not interchangeable with ordered wave selection. [Float](../../tests/cuda/nvvm-fp32-minmax-order.slang), [Double prefix](../../tests/cuda/nvvm-fp64-prefix-minmax-order.slang)                                    |
| Masked Half MIN/MAX                               | Separately qualified raw-half transport and ordered selection; finite exclusive seeds ±65504 | Infinity/NaN/signed-zero behavior follows exact seed and source order. Matrix prefixes remain separate. [Half](../../tests/cuda/nvvm-fp16-masked-minmax.slang)                                                                               |
| QuadAny/QuadAll                                   | Core composition with four unconditional source-lane reads; GPU                              | Complete source quads and matching shuffle sequences required; standalone requirement markers, partial quads and SPIR-V active-only semantics are not admitted. [Quad votes](../../tests/cuda/nvvm-quad-votes.slang)                         |
| Clock/clock64                                     | Live side-effecting per-SM cycle observations; relational GPU oracle                         | Wrapping counters, not wall time, cross-SM ordering or memory fence. Exact timestamp comparison is invalid. [Clock observations](../../tests/cuda/nvvm-clock-observations.slang)                                                             |

Public wave primitives now select eight named registry calls; raw payload decomposition and
vector/matrix composition live in core bodies. Nine semantic tags, six numeric IDs
(16/19/20/21/22/23), and their obsolete compound text routes are removed. The existing shuffle-width
and Float64 aggregate tests cover payload transport and shared mask reuse. The
[core wave edges](../../tests/cuda/nvvm-core-waves.slang) cover exited lanes named in the original
mask, identical/different NaN payloads, signed zero, 64-bit high-word differences and aggregate
equality. Matching/equality naturally extends to narrow and wide raw payloads; this does not change
CUDA's pre-existing match-all implementation of `WaveMaskMatch`, which differs from NVVM match-any
for unequal payloads.

Hardware snapshots use effectful canonical `WaveGetConvergedMask` (stable IR 909), retaining the
convergent side-effecting operation 79. Logical active-mask synthesis remains separate. The compact
edge fixture checks that snapshots exclude exited lanes and contain the observing lane; it does
not equate hardware participation with a source-level branch predicate. All-equal compares raw
match intersections with a participating ballot and evaluates every component collective.
Masked scan/reduction, quad and rotation algorithms now live in core compositions with their
existing ordering contracts. One generic fold shares control flow across seven algebras and three
modes. Sum/product retain Int32/UInt32/Float32/Float64 admission; bitwise retains Int32/UInt32;
min/max retains integer 8/16/32/64 and Half/Float32/Float64. Double singleton payloads and negative-zero
sum seeds, Half finite exclusive seeds and source-ordered floating min/max remain unchanged.
Numeric 15/17/43/44/45/48 are retired; only canonical ballot/match/hardware-mask 18/71/79 remain.
Scalar all/any now uses canonical Boolean conversion, retaining NaN=true and either zero=false.
Standalone quad requirement markers and the removed complete CUDA helper bodies still reject.

Core synchronization helpers directly name `llvm.nvvm.barrier0`, `llvm.nvvm.membar.cta` and
`llvm.nvvm.membar.gl` in their NVVM branches. Provider and compiler units cover canonical void
results, malformed signature rejection before module creation, retained effects and LLVM attributes.
The [synchronization fixture](../../tests/cuda/nvvm-synchronization-effects.slang) exchanges shared
values across two warps through non-inlined helpers, with two runtime loop iterations and an
independent count/error/checksum oracle of 128/0/4544. Its PTX checks retain both fence scopes and the
barrier. The [memory-only fence fixture](../../tests/language-feature/execution-model/group-memory-barrier-no-sync-emission.slang)
requires `membar.cta` without `bar.sync`. This preserves existing barrier scopes, including the
subgroup and WithGroupSync mappings. It does not qualify arbitrary transitive helper convergence:
LLVM convergence metadata is retained on the intrinsic but is not propagated to helper declarations.

## Texture, surface and descriptor contracts

| Region                                  | Current contract / evidence                                                                                                        | Limits / regression                                                                                                                                                                                                                                                                                           |
| --------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Sampled textures                        | Typed Sample/Level/Fetch/Gather operations with unchanged provider contracts; focused compile/runtime evidence                     | Full texture API and every shape/offset/format not implied. [Sample boundary](../../tests/cuda/nvvm-sampled-texture-unsupported.slang), [fetch boundary](../../tests/cuda/nvvm-texture-fetch-unsupported.slang)                                                                                               |
| Surfaces                                | Typed physical accesses; explicit static Half conversion; masks preserve untouched bits                                            | Byte-addressed access and element-count geometry differ. [Float surface](../../tests/cuda/nvvm-native-float-surface.slang), [formatted provenance](../../tests/cuda/nvvm-formatted-surface-provenance.slang), [integer boundary](../../tests/cuda/nvvm-native-integer-surface-unsupported.slang)              |
| Read-only texture descriptor conversion | Resource↔descriptor↔uint64 identity and UInt2 low/high word transport for accepted read-only texture families                      | Buffer descriptors contain pointer/count and do not inherit integer-handle conversion. [Texture descriptors](../../tests/cuda/nvvm-texture-descriptor-conversion.slang), [buffer negative](../../tests/cuda/nvvm-texture-descriptor-buffer-unsupported.slang)                                                 |
| Selected non-mip dimensions             | Typed UInt32 spatial queries for 1D/2D/3D/Cube and separate view-relative layer counts for 1DArray/2DArray; int/uint/float outputs | Other array counts, requested mip and total/view level counts remain excluded. [Dimensions](../../tests/cuda/nvvm-texture-dimensions.slang), [array counts](../../tests/cuda/nvvm-texture-array-layer-counts.slang), [query negatives](../../tests/cuda/nvvm-texture-query-unsupported.slang)                 |
| Undefined ordinary sampler              | Qualified CUDA placeholder semantics                                                                                               | Comparison samplers and arbitrary undefined resources remain excluded. [Sampler](../../tests/cuda/nvvm-undefined-sampler.slang), [comparison negative](../../tests/cuda/nvvm-undefined-comparison-sampler-unsupported.slang), [resource negative](../../tests/cuda/nvvm-undefined-resource-unsupported.slang) |

Core producers now own texture operation identity and output composition. Existing Sample and
logical ImageLoad/ImageStore are joined by stable operations for explicit level,
integer fetch, ordinary gather, spatial size and scalar array layer count. Full resource types reach
preflight; sampler validation remains required, while the provider still uses CUDA texture-object
state. Gather offsets remain ignored. Non-mip Texture1DArray/Texture2DArray layer counts use a
distinct scalar UInt32 query mapped to `txq.height`/`txq.depth` with the real array descriptor
preserved. Spatial rank remains one/two. Integer and Float32 outputs use ordinary numeric casts; other array-count bodies retain the previous zero
fallback. Mip, multisample and writable-resource admission remain unchanged. Public direct
RWTexture2DArray.Store gains the same native32 path already admitted by canonical image stores.
Float3 sampling, extra access modes and query-level admission remain excluded.

All active semantic tags and resource-text recognizers are retired. Deprecated AST/IR serialized
slots remain inert under module 43; ABI 46/container 2 are unchanged. Exact removed-tag/text tests
reject before provider creation, and existing named explicit-operand tests keep that syntax live.

Texture research distinguishes base geometry, selected mip geometry, layer/cube count and total
allocated/view levels. The CUDA source producer currently loses some of those semantics. Agreement
with NVRTC can therefore preserve the same error. A texture object and a surface handle are not
interchangeable, and opaque descriptor bytes cannot be inspected as an undocumented metadata API.
For correctly bound non-mip 1D/2D arrays, layer count follows the bound view, consistent with
[Direct3D resinfo](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/resinfo--sm4---asm-)
and [Vulkan image queries](https://docs.vulkan.org/spec/latest/chapters/images.html#spirvenv-image-query).
CUDA C++ currently returns zero for this count, an intentional comparison discrepancy rather than
the NVVM oracle. The render-test input producer currently treats `arrayLength=1` as non-array;
this separate host limitation is not repaired with a shader fallback. CubeArray counts remain
outside the qualification. NVVM O0/O3 runtime covers full arrays with 2/3/5 layers and all three
output types. A raw-handle Slang O3 kernel additionally returns 11×7×5 for the full view and
11×7×3 for a restricted view of the same allocation on CUDA12.9/SM80. Explicit view descriptors and
untouched guards were independently checked. Null-view descriptor getter garbage is retained as an
API anomaly and is never used as query metadata. The same output families now also cover 1D arrays
with 2/3/5 layers; a raw Slang O3 kernel returns width/count 11/5 and 11/3 for independently validated
full/restricted views of one layered allocation.

A separate four-case probe retained two CubeArray mismatches: explicit views of an 8×8×30-face
allocation with requested layer ranges 0..29 and 6..23 returned query depth 30/18, against intended
public counts of 5/3 cubes. All APIs, echoed descriptors and guards passed. Echoed range fields do
not establish their semantic units. Earlier null-view cube queries returned cube counts; these
observations do not justify a universal division by six. A subsequent content probe filled each
physical face with `100*cube+face`. Null-full and explicit 0..4 views returned count 5 and sampled
all five source cubes correctly. Explicit 1..3 returned count 3, but +X/+Z samples were
`1,5,101,105,201,205` instead of `100,104,200,204,300,304`. Guards and unused output slots survived.
Thus matching query size does not establish correct cube selection. Restricted cube-view
interpretation remains unresolved; both experiments and failed hypotheses are retained.

The observed driver lookup failures for `txq.array_size` and `txq.num_mipmap_levels` are specific to
the qualified stack; `txq.level.width` loaded and executed. No full API repair is implemented.

NVVM surface legalization supports the existing native 32-bit signed/unsigned/Float32 scalar, two-
and four-channel transfers in 1D/2D/3D and 1DArray/2DArray shapes. Native Half and annotated Half
storage with Float32 shader values support the same 1D/2D/3D and 1DArray/2DArray shapes. Matching Float32 uses Float32
payloads directly. Static component masks and in-range dynamic scalar indices update physical channels without
re-encoding untouched NaN payloads. Dynamic indices use the existing non-atomic whole-texel merge;
there is no new out-of-range lane guarantee. Byte-X scaling and conversion are explicit IR; no runtime format discovery is added.

Explicit `r/rg/rgba` signed/unsigned 8/16-bit formats additionally use matching `int/uint`32
shader values in 1D/2D/3D and 1DArray/2DArray surfaces. Loads sign- or zero-extend; stores clamp in the logical
32-bit type before narrowing. This follows the [D3D narrower-integer conversion contract,
§3.2.3.13](https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm).
[Vulkan integer image reads](https://docs.vulkan.org/spec/latest/chapters/images.html#images-reads)
agree on load interpretation; out-of-range Vulkan storage-image encoding equivalence has not been
established. Normalized formats and mixed signedness remain excluded.
The static annotation owns physical interpretation; opaque handles do not infer runtime formats.

The [physical-storage harness](../../extras/validate-nvvm-surfaces.py) independently checks 106 cases,
including exhaustive Half loads, conversion boundaries, direct literals, written NaN classification,
nonzero guards and zero-boundary accesses. Native signed/unsigned32 cases exercise 1D/2D scalar,
two- and four-channel loads, whole stores and component stores with exact integer bits. Four mixed
cases bind eight separate native Float32/r16f/rg16f/rgba16f source/result resources, checking both copy
directions and untouched channels. The last full checkpoint passed 81 cases in each NVVM mode and retained two dynamic-index
failures per mode. Focused dynamic-component qualification now covers those two cases plus two
static controls at O0/O3, with unchanged physical oracles and guards. The dynamic cases exercise
all four lanes in 2D Float32/Half storage and exact untouched NaN payloads; signed/two-lane shapes
have compiler-unit coverage, not fresh physical runtime coverage. NVRTC's full checkpoint retains
52 passes, 20 compile failures and 11 rounding mismatches.
Two additional [layered cases](../../tests/cuda/nvvm-surface-physical-layered.slang) group all nine
native32 kind/width combinations in 1D arrays and isolate 2D-array coordinate ordering. Distinct
resource/layer/coordinate/lane bits, independent stores and complete host readback prevent a matching
wrong load/store address from hiding corruption. The original 83 case/oracle identities remain
unchanged. Explicit array role selects layered allocation and copy depth, even for a singleton;
CPU checks cover that role without claiming singleton GPU qualification.
Four [Half-array cases](../../tests/cuda/nvvm-surface-physical-half-layered.slang) cover both array
ranks, native Half and formatted Float32 values, widths 1/2/4 and whole/static-component updates.
Fourteen independently read-back resources distinguish raw copies, narrowing, widening and source
marker writes. NVVM O0/O3 passes all eight cells; four CUDA cells retain missing layered conversion
helpers and invalid component-subscript compilation. Converted NaNs require class; native copies,
untouched channels and layer guards retain exact bits. No new dynamic-array runtime claim is made.
Two [Half-volume cases](../../tests/cuda/nvvm-surface-physical-half-volume.slang) cover the same
representations and widths with ordinary XYZ coordinates. All four new NVVM cells pass; volume
allocation/copy depth is separate from the layered flag. CPU and report checks reject incorrect
depth, height and array roles. CUDA fails both entries because their shared helper emits invalid
component subscripts; this does not establish failure of CUDA whole-volume operations generally.
Six [integer-format cases](../../tests/cuda/nvvm-surface-physical-integer-formats.slang) cover all twelve
explicit signed/unsigned 8/16-bit formats with whole/static/dynamic writes at both non-array ranks.
All twelve NVVM O0/O3 cells pass. Loads are recorded in twelve separate native32 arrays before
independently generated store edges overwrite the source arrays. Representative signedness patterns,
each selected lane's sixteen store edges, complete guards and unchanged component bits are exact.
These tests are bounded patterns, not exhaustive integer decode. All six CUDA comparison cells fail
compilation because integer surface-conversion helpers are missing; component generation also has
existing subscript limitations. Their failure identities are unchanged from the before run.
Nine [integer spatial cases](../../tests/cuda/nvvm-surface-physical-integer-spatial.slang) extend the
same twelve formats and three write styles to 1DArray/2DArray/non-array3D. All eighteen new NVVM
cells pass. Explicit array flags and volume depth, asymmetric Y/Z extents, guarded outer planes,
coordinate-varying input bits and independent Z-phased stores exercise the geometry separately
from conversion. CPU checks reject a bounded matching load/store plane permutation. All nine CUDA
comparison cells retain missing integer read-conversion helpers; the numerical policy is unchanged.
The recurring checkpoint replays physical readbacks and compares its complete inventory against the
[current baseline](../../issue-nvvm-backend/accepted-baseline.json), preserving known failures.
That full baseline still owns its original 249 cells; the sixty-nine added array/volume/integer cells require explicit
reviewed adoption at the next full checkpoint.
[Ordinary Half conversion](../../tests/cuda/nvvm-half-narrow-conversion.slang)
has a separate three-mode regression. Half stores use RN-even; this differs from the existing NVRTC
formatted store's observed truncation. NVRTC component writes (including dynamic indexing), user resource-helper format provenance, three-channel transfers and additional packed/normalized formats
remain outside this qualification. The pass does not add layered helper writes or general aliases.

The unannotated surface contract requires a matching physical channel width, count and scalar
interpretation. A focused host-readback experiment binds the same `RWTexture1D<int4>` kernels to
signed8×4 and signed32×4 CUDA arrays. Whole stores fail the packed physical oracle in NVRTC O3 and
NVVM O0/O3; component stores fail in both NVVM modes, while NVRTC component source does not compile.
All five matching signed32 controls pass. Every physical byte and six neighboring texels are checked;
these are bounded 1D observations, not qualification of arbitrary formats or array shapes.
The original `compute/texture-subscript.slang` binds RGBA8Sint but checks results through the same
shader access mapping. Its historical NVVM passes do not establish packed-format correctness.
The CUDA component-source gap and packed-format limitation remain separate open obligations in
[focused evidence](../../issue-nvvm-backend/focused-evidence.json); no source fix or format conversion
is claimed by this investigation.

## Language composition and application evidence

| Region                                                                            | Evidence currently available                                                                                                             | Limit                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| --------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Dynamic dispatch                                                                  | Permanent simple and nested AnyValue records with live runtime selection/mutation                                                        | Concrete record roles and payload capacity remain bounded. [Original substandard case](../../tests/compute/dynamic-dispatch-substandard-float.slang), [nested regression](../../tests/cuda/nvvm-nested-substandard-dynamic.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| Errors and generic throwing calls                                                 | Runtime-dependent scalar/aggregate propagation, catches, generic-specialized witness path; permanent three-mode GPU tests                | Generic specialization is not an existential throwing-witness ABI. [Runtime errors](../../tests/cuda/nvvm-runtime-errors.slang), [throwing witness](../../tests/cuda/nvvm-runtime-throwing-witness.slang)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| Inheritance and initialization                                                    | Focused unchanged-source three-mode qualification of struct inheritance, derived initializer lists, 16-bit defaults and scoped constants | Some expressions/calls/arrays can fold; no IR survival, arbitrary inheritance depth or nonintegral Half claim. [Inheritance](../../tests/language-feature/inheritance/struct-inheritance.slang), [derived initialization](../../tests/language-feature/inheritance/derived-struct-init-list.slang), [16-bit defaults](../../tests/language-feature/initializer-lists/default-init-16bit-types.slang), [constants](../../tests/language-feature/constants/static-const-in-struct.slang)                                                                                                                                                                                                                                                                                                                                       |
| Accessors, generics, variadics, constrained extensions, lambdas, tuples and defer | Focused unchanged-source experiments plus main-corpus selection                                                                          | Passing isolated examples does not establish all feature intersections. Selected membership and semantic coverage are different measurements.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| Material shaders                                                                  | Both registered entries × three modes compile/assemble and execute with two live synthetic textures and independent scalar oracles       | 65 active records/entry/mode and 63 untouched guards. Eval checks 260 components; sample checks 455 floats/65 flags, selected-layer throughput, coherent arithmetic candidates and exact rejection zeros. Fixed synthetic inputs only; no original assets, live LUTs or sampling-distribution claim. Device-event evidence has its own scoped row below. [Runtime validator](../../extras/validate-nvvm-material-runtime.py). [Material manifest](../../issue-nvvm-backend/complex-corpus.manifest.json)                                                                                                                                                                                                                                                                                                                     |
| Synthetic material device events                                                  | Both entries × two counts × three modes × two reversed rounds; 216 measured launches and 72 warmups pass complete output checks          | At N1,048,577, fresh NVRTC O3/NVVM O3 time ratios are 8.93–8.97 eval and 6.26–6.30 sample across the two rounds on the qualified L4 (pooled medians: 8.95×/6.29×). Tiny hot textures, periodic inputs and inter-launch correctness transfers only; no application performance or causal claim. Small NVVM O3 cells fail the 0.1 ms throughput gate. An earlier eval-only diagnostic, not rerun in this refresh, removes 65 proven never-read PTX stores: reassembly eliminates its stack and reduces NVRTC time about 8.15× with exact outputs. This is not a production pass or a sampling result. [Runner](../../extras/measure-nvvm-material-runtime.py), [CPU contracts](../../issue-nvvm-backend/test-nvvm-material-measurement.py), [protocol](../../issue-nvvm-backend/RESULTS.md#material-device-event-measurement). |

The material validator also qualifies a separately named `linear-filtering` input profile in six
GPU cells. Four dyadic UV locations exercise horizontal, vertical and bilinear blends and a footprint
across both wrap seams; encoded color is interpolated before sRGB decoding. Each cell checks 65 active
records and 63 guards. Both entries retain independent oracles, fixed tolerances, and sample-wide
arithmetic hypotheses. Six fresh default-profile cells preserve exact input/oracle/output bytes.
Integer-offset equivalence is observed at the first location; arbitrary UV quantization, assets,
LUTs and timing remain outside this qualification. The original device-event protocol uses centers.

A separate [imported mini-LUT fixture](../../tests/cuda/nvvm-material-mini-lut.slang) executes the
unchanged material library's 16×16 mini-microfacet interpolation and reflection compensation in all
three modes. Twelve runtime queries cover corners, cosine saturation, asymmetric interiors and both
sides of the compensation denominator floor. Each mode checks 48 numeric components against an
independent rational oracle, exact zero-knot bits, completion and a guard. The table is synthetic and
Fresnel is fixed; original assets, larger LUT families and LUT execution in the registered analytic
graph remain unqualified. This fixture uses the standard renderer, with no material-driver change.

The [imported dielectric fixture](../../tests/cuda/nvvm-material-dielectric.slang) separately qualifies
runtime reflection/transmission evaluation and PDFs with the existing no-compensation policy.
Twelve queries cover normal combined/single/disabled modes and off-axis anisotropic x/y directions.
Independent geometric equations check 48 float components per mode; raw IEEE readback and permanent
finite/error masks, exact disabled zeros, completion and guard pass all three modes. The fixed budget
is `1e-6 + 1e-5*abs(expected)`. This does not qualify sampling, TIR, backside handling, absorption,
retroreflection, compensation, arbitrary inputs or performance. The registered graph remains
reflection-only; its code and assets are unchanged.

Material-derived aggregate/indexing coverage also runs through Slang in all three modes. The
[bounded and literal controls](../../tests/cuda/nvvm-aggregate-index-material.slang) retain initialized
unused fields, a receiver snapshot and a nested stack copy. A separate
[copy-then-mutate witness](../../tests/cuda/nvvm-aggregate-index-snapshot.slang) checks saved and live
slots independently, including all vector lanes. Six GPU cells check 96 exact integer components;
these two focused sources remain outside the main corpus denominator. This qualifies behavior, not
identical generated code or performance relative to the CUDA reduction.

The eval diagnostic also has a source-derived CUDA reduction with independent static address proof.
Eight bounded-index variants retain 27 never-read store instructions and a 584-byte stack; eight
constant-index variants have no local memory. The other three source factors do not change PTX.
Three full-material force-inline controls reproduce the original PTX exactly. All 19 candidates
compile and assemble; none has a reduced-kernel GPU correctness or timing qualification.

The [current focused evidence](../../issue-nvvm-backend/focused-evidence.json) retains the inheritance/
initialization and mixed-record-array boundary outcomes separately from the full baseline.

The historical suite inventory counted 4,639 active files and 10,834 executable directives, including
1,852 compute-oriented sources and 795 explicit-CUDA sources. The current main selection has 576
source files and 580 cases/1,740 mode cells. Its 31.1% compute-source and 65.0% explicit-CUDA-source
selection figures (576/1,852 and 517/795) are historical inventory ratios, not percentages of language
semantics covered.
Later focused cases remain outside that main denominator unless explicitly enrolled.

Field/index address recipes are shared by pointer validation, ordinary memory planning and
provider emission. Nested borrowed-vector coverage checks dynamic lane access across a mutation,
scalar alignment and the absence of invariant-load metadata for Generic Read borrows. Compact
parameter-group loads retain their independent representation, alignment and invariant metadata.
These checks preserve existing support boundaries. Parent proofs and raw-view access are retained
directly; pointer spelling alone cannot grant a child writable resource access.

## Current compilation observations

The current results refresh uses the accepted optimized compiler on an AMD EPYC 7R13 host with
four visible logical CPUs; one compiler process runs at a time. The standalone release cohort has
504 cases/500 sources and 76 explicit exclusions. All 1,512 variants compile and assemble; all
9,072 measured compilations and 3,024 warmups preserve the qualified PTX.

| Cohort    | Cases | NVRTC O3 / NVVM O3 | NVRTC O3 / NVVM O0 |
| --------- | ----: | -----------------: | -----------------: |
| Frozen    |   406 |              1.47× |              1.51× |
| Discovery |    98 |              1.41× |              1.49× |

These are geometric means of per-case median fresh-process time ratios, using six samples per cell
and warm caches. NVVM O3 has lower observed medians in 400/406 frozen and 94/98 discovery cases;
NVVM O0 does so in all selected cases. Equal `-g0` policy avoids asymmetric renderer debug options.
Original runtime contracts remain authoritative; timed release PTX is compile/assembly-qualified,
not separately GPU-qualified. No general application compilation or statistical-significance claim
follows. See [protocol](../../issue-nvvm-backend/RESULTS.md#standalone-corpus-compilation) and the
`compilation-performance` feature in [current evidence](../../issue-nvvm-backend/focused-evidence.json).

Material compilation uses 18 samples per cell. Eval medians are 1.381/1.376/1.292 seconds and
sample medians 1.401/1.473/1.336 seconds for NVRTC O3/NVVM O3/NVVM O0. Thus the broad-corpus gains
do not transfer uniformly to these larger entries. Assembly and inclusive Slang phases remain
separate measurements. The 12-shader quality subset supplies fresh static resources for 36 cells;
its single compile observations and code sizes are not GPU-performance evidence.

Original-input GPU dispatch measurements now attempt all 452 frozen and 128 discovery cases in
NVRTC O3, NVVM O3 and NVVM O0 with explicit `-g0`: two reversed rounds, three warmups and nine
samples per mode/round, reset resources before every launch, and original comparison oracles.
Complete three-mode comparisons cover 449 frozen and 117 discovery cases; 30,618 accepted samples
span 3,402 measured mode-round cells. The 78 exclusions retain 72 instances of the existing 36 gaps
and six new repeatability failures in the copyable-context fixture. The latter returns typed uint32
failure sentinels, not aggregate padding; both backends leave its bare static aggregate uninitialized.
The ordinary full checkpoint still has all 1,740 original outcomes unchanged.

Of 566 complete cases, 557 are below the prespecified 0.1 ms ratio cutoff in at least one mode.
Eight of the nine longer cases exercise wave/min/max and show 1.76–2.42× higher NVVM O3 intervals
than NVRTC O3; FP8 scalar transport is near parity at 1.03×. These are original correctness workloads,
including shader-side checks. CUDA events include RHI parameter upload and host enqueue gaps;
reset, readback, compilation and encoding are outside. Clocks are not locked. Short observations
are retained without ratios; the cutoff is policy, not calibrated resolution. No aggregate shader
throughput or application-performance claim follows. See the
[runner](../../extras/measure-nvvm-corpus-runtime.py),
[CPU contracts](../../issue-nvvm-backend/test-nvvm-corpus-runtime.py),
[harness contracts](../../extras/test-cuda-dispatch-profile.py),
[protocol](../../issue-nvvm-backend/RESULTS.md#original-input-corpus-dispatch-timing) and
`corpus-dispatch-performance` in [current evidence](../../issue-nvvm-backend/focused-evidence.json).

Fresh original-input code capture covers all 580 cases and three modes: 1,704 qualified cells,
36 retained gaps, and 567 complete O3/O3 comparisons. Offline SM89 assembly finds 194 cases
with identical bytes in every named executable section, 0 additional normalized-PTX matches,
46 similar static profiles, 327 different profiles, and 13 incomplete comparisons.
Similarity is a triage heuristic, not performance proof. NVVM uses fewer/equal/more hardware
registers in 103/402/62 paired cases.
The narrow masked min/max slowdown coexists with smaller PTX and fewer offline registers:
eligible-mask fast paths and aggregate traversal differ. Floating min/max already uses trees,
but retains mode selection within loops and repeated component traversal. Interface-return dispatch
shows a separate tag/control-flow simplification opportunity (33→94 PTX, 40→104 SASS instructions);
its short fixture does not support a runtime regression claim. Static metrics use fresh captures
and offline ptxas, not historical timed PTX or recorded driver-JIT machine code. No new timings,
compiler changes or broader feature qualification are implied.

See the [capture tool](../../extras/capture-nvvm-corpus-code.py),
[analyzer](../../extras/analyze-nvvm-corpus-code.py),
[protocol](../../issue-nvvm-backend/RESULTS.md#original-input-corpus-code-quality) and
`corpus-code-quality` in [current evidence](../../issue-nvvm-backend/focused-evidence.json).
The current presentation includes every case, resource/instruction tables and six source/PTX/SASS
case studies. Existing semantic qualifications and failure histories remain unchanged.

## Known gaps and evidence boundaries

- The current main baseline retains 36 unresolved cells: three column-major host-packing mismatches
  and 33 infrastructure/preflight outcomes. Keep original bytes, identities and expected outputs.
- Canonical multi-lane vector updates now emit legal component assignments in CUDA/C++/WGSL.
  [Snapshot coverage](../../tests/compute/swizzle-set-snapshot.slang) checks overlapping Half lanes,
  noncontiguous updates, untouched lanes and one effectful Float32 helper call in three GPU modes.
  [Source coverage](../../tests/compute/swizzle-set-source.slang) checks all five source targets;
  HLSL/GLSL retain multi-lane syntax. WGSL has source checks only. The original Half-vector corpus
  test is correct in all three modes with unchanged input and oracle.
- The separate Half-vector noinline helper failure survived the source-emission repair: NVVM O3
  omitted argument/result transfers while O0 and NVRTC were correct. Parameter-only, result-only,
  combined and exported probes isolated the boundary with identical O0/O3 provider LLVM inputs.
  Physical integer lane transport now repairs this boundary; the original wrong-output observations
  remain preserved in focused evidence. Ordinary arithmetic evidence alone does not qualify an ABI.
- The qualified column-major `float3x2` uses CUDA column stride 12 and size 24. The permanent
  [compact-column test](../../tests/cuda/nvvm-column-major-compact.slang) checks all six elements
  plus multiplication in three GPU modes (24 exact Float32 components), with fresh 24-byte layout
  reflection. Graphics-packed eight words produce `11,1`; the CUDA-packed six words produce `11,22`.
  The original three discovery mismatches remain unchanged. This does not establish that arbitrary
  graphics host layouts can be reused for CUDA or expand the main corpus denominator.
- Three focused integer nested-array cases still produce wrong output under NVRTC O3 while the
  corrected NVVM modes pass. They are outside the main-corpus outcome count.
- Local fixed mixed-record arrays now have permanent transport/snapshot GPU coverage. Exactly two
  historical mixed-local NVVM cells transition from preflight refusal to correct output. Internal
  value-array parameters pass a separate two-cell NVVM qualification. A bounded three-mode reference
  fixture checks mutable and readonly forwarding, source array returns via OutParam, snapshots,
  both slots and BF2 lanes using four independent bit-pattern seeds. Readonly observations before
  and after caller mutation keep read permission separate from immutable memory. Native array
  results, external roles and guarded wrappers remain excluded. Prior integer controls, corruption rejection and NVRTC
  failure history retain their original identities. Neither qualification establishes large-array
  scalability.
- The result classifier previously mistook `nvrtc` in a source path for an NVRTC compilation
  diagnostic. The parser now requires a downstream compiler identity prefix and accepts the actual
  `error :` producer form. The original failed attempt/adjudication remains in focused evidence;
  its output mismatch never becomes a passing shader.
- Toolkit/architecture compile-and-assembly matrices are separate from physical-device execution.
  Historical CUDA 12.9 SM70/80/90 and CUDA 13 SM80/90 results are not fresh qualification of every
  later feature. The manually defined container workflow was not dispatched in that evidence.
- OptiX, source-level debug support, relocatable device code, device LTO, dynamic parallelism and
  device syscalls remain separate tracks. No general cooperative or autodiff support claim follows.

Further feature work should select a coherent role/operation combination and preserve neighboring
negative contracts. A diagnostic advancing to another unsupported instruction is research progress,
not implemented support. New evidence updates this matrix and permanent tests where appropriate;
chronological plans, reports and repeated raw outputs belong in working artifacts or Git history.
