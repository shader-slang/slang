# Slice 145 report: canonical ephemeral values

## 1. Motivation

The Slice 144 census left ten healthy-MVP workloads in the broad
`residual-target-marker-or-undefined-value` bucket. That count looked like the largest remaining
root cause, but decomposing it by canonical shape showed five distinct meanings:

| First shape | Healthy-MVP workloads | Meaning |
|---|---:|---|
| `DebugNoScope` | 4 | inliner-produced debug-scope closure |
| `LoadFromUninitializedMemory` | 3 | SSA value whose concrete value may be selected by optimization |
| `getStringHash` | 1 | compile-time hash of an already-validated literal |
| `RequireComputeDerivative` | 1 | target execution requirement |
| `RequirePrelude` | 1 | target-language source requirement |

The first eight workloads share a code-generation boundary: their canonical IR must be consumed
without reconstructing a CUDA source expression. The last two do not. Ignoring either target
requirement would merely move past the first diagnostic while losing semantics.

Consider the undefined-value producer exercised by the legalization regressions:

```slang
[ForceInline]
float chooseScalarNonVar(int seed)
{
    for (int i = 0; i < 2; ++i)
    {
        if (i == 0)
            continue;
        return float(seed + i);
    }
    return float(seed);
}
```

After control-flow legalization, SSA construction needs an initial value for the loop-carried
result and emits `LoadFromUninitializedMemory`. Its IR contract permits an optimization to select
one concrete value for that instruction. The direct backend instead rejected the valid producer
before it could reach its initialized path.

The other two examples are similarly canonical. Inlining inserts `DebugNoScope` when leaving an
inlined debug scope, even when the target emits no debug information. GPU type inlining and
`checkGetStringHashInsts` reduce:

```slang
int hash = getStringHash("Hello World!");
```

to `getStringHash(IRStringLit("Hello World!"))`. Established emitters consume those forms, while
direct NVVM rejected them.

## 2. Proposed solution

Add one compiler-owned ephemeral-value resolver shared by shape preflight, SSA availability
validation, and emission:

- accept `LoadFromUninitializedMemory` only for the established finite copyable-value algebra;
- choose a concrete all-zero value through existing generic scalar constants and recursive
  vector/aggregate construction, then retain its provider handle in the SSA value map;
- accept exact void `DebugNoScope` as a semantic-free marker while direct debug output is absent;
- accept `getStringHash` only for signed i32 with one `IRStringLit` operand and hash the literal
  bytes with `getStableHashCode32`;
- leave `RequirePrelude`, `RequireComputeDerivative`, poison, unsupported value types, and all other
  markers on deterministic preflight diagnostics.

This is principled for each accepted producer. The undefined-value contract explicitly allows one
selected value, and using the value map makes all consumers of one instruction observe the same
choice. Debug scope closure has no executable meaning without debug output. Literal hashing uses
the semantic source of truth already proven by the GPU lowering pipeline. No fixture path, source
name, arbitrary operand search, or syntax reconstruction participates.

Provider ABI revision 30 already has every required generic operation. A new LLVM `undef` callback
would add an interface solely to represent a value the compiler is allowed to choose concretely;
it is unnecessary. The isolated LLVM 14 provider therefore remains unchanged.

## 3. Change summary

- `source/slang/slang-emit-nvvm.cpp`
  - adds exact ephemeral-value classification;
  - recursively materializes one selected zero value for scalar, vector, fixed-array, and nonempty
    struct copyable types;
  - consumes `DebugNoScope` without provider output;
  - computes validated literal hashes from string bytes and preserves their exact i32 bits;
  - shares the resolver across both validation passes and emission.
- `tools/slang-unit-test/unit-test-nvvm-support.h` and
  `tools/slang-unit-test/unit-test-nvvm-emitter.cpp`
  - add focused fake-provider sources and a contract test proving chosen Float32 zero, ignored
    inline debug scope, and the stable hash value `1840786589` for `"Hello World!"`.
- Eight existing workload files
  - add explicit direct-NVVM O0 and O3 differential lanes.
- `issue-nvvm-backend/census.slice-145.tsv` and
  `issue-nvvm-backend/census.slice-145-clusters.json`
  - retain the complete fixed 452-workload result and post-slice root-cause distribution.
- `docs/design/nvvm-backend.md` and
  `docs/design/nvvm-backend-capability-ledger.md`
  - record the durable representation contract, metrics, denominator, and remaining target
    requirements.
- `issue-nvvm-backend/plan.slice-145-canonical-ephemeral-values.md`
  - records the bounded execution contract, producer audit, decisions, validation, and outcome.

The eight promoted workloads are:

1. `bugs/gh-4533`;
2. `bugs/legalize-defuse-no-zero-init-non-var-primitive`;
3. `bugs/legalize-defuse-no-zero-init-non-var`;
4. `bugs/legalize-defuse-no-zero-init`;
5. `bugs/metal-return-value-lost`;
6. `bugs/string-inline`;
7. `language-feature/dynamic-dispatch/special-members-setter`;
8. `language-feature/generics/variadic-type-pack-expand-tuple`.

## 4. Concepts and vocabulary

- **Ephemeral value:** This slice's local term for a canonical post-link instruction that codegen
  consumes without producing a source-level target expression. It does not mean the instruction
  lacks semantics.
- **Chosen undefined value:** One concrete same-typed value selected for a
  `LoadFromUninitializedMemory` instruction, as permitted by that instruction's IR contract.
- **Copyable-value algebra:** The existing finite direct-NVVM type set: selected scalar/vector
  leaves, fixed arrays, and nonempty structs recursively containing them.
- **Healthy MVP denominator:** The 427 MVP workloads whose NVRTC O3 reference succeeds on this
  machine. It excludes three native-reference infrastructure failures and extension-tier rows.
- **Selected regression prefix:** The focused direct-NVVM unit/integration suite. Its 423/423 score
  guards established behavior but is not the coverage denominator.

## 5. Process report

### The corpus bucket was split by semantics before implementation

The complete Slice 144 TSV was filtered to healthy MVP rows with the residual cluster. Exact first
shapes gave the 4/3/1/1/1 split above. This prevented a tempting but invalid implementation that
would simply skip every instruction called a marker.

`RequireComputeDerivative` is consumed by GLSL/SPIR-V emitters to establish compute derivative
execution behavior. `RequirePrelude` injects text referenced by source-level intrinsic assembly.
Neither is metadata that direct LLVM emission may erase. They remain the two healthy-MVP residual
failures after this slice, with their original E52017 diagnostics.

### One concrete selected value preserves the undefined-value contract

`slang-ir-ssa.cpp` emits `LoadFromUninitializedMemory` when SSA construction requires a value not
proven initialized. `slang-ir-insts.lua` defines the exact contract: one occurrence may evaluate to
an arbitrary value of its type, and an optimization may replace every use with one selected value.

IR dumps of the three bounded workloads showed only:

- signed/unsigned i32 in `bugs/gh-4533`;
- Float32 in `legalize-defuse-no-zero-init-non-var-primitive`;
- `OutVec { float data[8]; }` in `legalize-defuse-no-zero-init-non-var`.

All are already in `isNVVMSupportedCopyableValueType`. `_resolveNVVMEphemeralValue` accepts that
canonical type, the second validation pass adds the instruction as one available SSA value, and
`_emitNVVMChosenUndefinedValue` recursively builds a zero of the same complete type. The finished
handle is stored once in `valueMap[inst]`; no use graph is traversed and no source initializer is
rebuilt.

Choosing zero is legal here even though two distinct undefined instructions are allowed to choose
different values: the contract permits, but does not require, different choices. A provider
`undef` callback was rejected because it would revise the ABI without a concrete expressiveness
gap and could also allow different choices at different LLVM uses unless an older-dialect freeze
representation were introduced.

The focused fake-provider source proves the Float32 zero constant. Real O0/O3 comparisons prove
the scalar integer, scalar floating, array, and struct paths across three undefined-value workloads.

### Debug scope closure is valid emitter input, not malformed upstream IR

`slang-ir-inline.cpp` emits `DebugNoScope` at the end of inlined scopes. That is a canonical marker,
not an accidental alternate spelling. `slang-emit-llvm.cpp` already consumes it only when LLVM
debug emission is active; otherwise it has no output. The direct NVVM backend currently emits no
debug information, so the resolver requires exact void `DebugNoScope`, the SSA pass does not add a
value, and emission performs no provider operation.

This layer owns the decision because it owns whether debug output exists. Moving the fix upstream
by stripping this one instruction would alter the shared linked IR solely for one emitter and
would not generalize to future direct debug support. Four unrelated workloads produced the marker
through normal inlining and all four pass at O0 and O3.

### Literal hashing uses the validated semantic operand

`slang-emit.cpp` performs GPU type inlining, eliminates dead residual helpers, and runs
`checkGetStringHashInsts` when non-essential validation is enabled. The direct resolver still
independently requires exactly one `IRStringLit`, so disabling that optional validation cannot
turn a malformed operand into memory reinterpreted as a string.

The established C-like, LLVM, SPIR-V, and WGSL emitters hash the literal character bytes. The first
prototype accidentally called the templated hash overload on the `UnownedStringSlice` object,
which hashed its pointer/length representation. Differential execution exposed this immediately:
NVRTC produced the Float32 bit pattern `4EDB7059`, while direct O0/O3 produced different negative
values. The implementation was corrected to pass `string.begin()` and `string.getLength()`. The
focused fake test fixes the exact integer hash at `1840786589`, and the real workload then compares
correctly in both modes.

The uint32 hash is converted to the signed provider integer with explicit modulo-2^32 arithmetic,
avoiding implementation-defined unsigned-to-signed conversion while preserving the exact i32 bits.

### Validation evidence and measured breadth

The final fixed census remains 448 eligible sources and 452 eligible workloads:

| Mode | Correct | Runtime mismatch | Slang preflight | Provider | Infrastructure |
|---|---:|---:|---:|---:|---:|
| NVRTC O3 | 449 | 0 | 0 | 0 | 3 |
| direct NVVM O0 | 365 | 8 | 72 | 7 | 0 |
| direct NVVM O3 | 370 | 8 | 72 | 2 | 0 |

Compared with Slice 144, direct O0 and O3 each gain exactly the eight bounded workloads. An
identity comparison finds zero previously correct losses. On the 427 healthy-MVP denominator:

| Measure | Slice 144 | Slice 145 |
|---|---:|---:|
| O0 correct | 355/427 (83.1%) | 363/427 (85.0%) |
| O3 correct | 359/427 (84.1%) | 367/427 (85.9%) |
| correct in both modes | 355/427 (83.1%) | 363/427 (85.0%) |

The post-slice healthy-MVP failure Pareto is:

| Root-cause cluster | O0 | O3 |
|---|---:|---:|
| aggregate/pointer/layout transport | 8 | 8 |
| helper ABI/type contract | 8 | 8 |
| other exact preflight shapes | 8 | 8 |
| common wave/reconvergence GenericAsm | 8 | 8 |
| function identity | 6 | 6 |
| O0-only unoptimized Half provider behavior | 4 | 0 |
| atomic/wave operation | 3 | 3 |
| generic-asm atomic | 3 | 3 |
| raw-buffer view/access | 3 | 3 |
| descriptor-handle runtime layout | 3 | 3 |
| residual target requirements | 2 | 2 |
| provider aggregate field pointer | 2 | 2 |

All remaining one-workload clusters stay in the checked-in TSV/JSON rather than being hidden by
the Pareto cutoff.

The selected regression prefix passes 423/423. All sixteen promoted direct lanes pass. Running
some entire promoted files also executed unrelated WGPU lanes; those exposed an existing invalid
bind-group-layout failure on this machine. The direct lanes themselves passed, and the fixed CUDA
census was unaffected.

Representative standalone metrics remain healthy:

| Workload gate | NVRTC O3 median / PTX | direct O3 SM70 median / PTX |
|---|---:|---:|
| resource + aggregate + helper | 390.0 ms / 8889 B | 270.3 ms / 919 B |
| parameter-block layout | 369.7 ms / 8839 B | 251.5 ms / 793 B |
| shared control + barriers | 372.9 ms / 9190 B | 255.2 ms / 1404 B |

End-to-end census compile/load/execute/compare times are respectively 5277/5171 ms, 4919/5006 ms,
and 5112/5022 ms for NVRTC O3/direct O3; these are not kernel-only runtime measurements. Direct O3
PTX assembles under CUDA 12.9 for SM70, SM80, and SM90. Runtime comparison uses the local RTX 5090
(SM120, driver 610.62). CUDA 13 and physical SM70/SM80/SM90 runtime workers remain explicit gaps.

### Self-review inventory

- `_resolveNVVMEphemeralValue` survives. It recognizes three canonical op/type relations and is
  shared across every compiler phase that consumes them.
- `_emitNVVMChosenUndefinedValue` survives. The exact IR contract permits a concrete choice, the
  recursion follows existing canonical type structure, and scalar plus aggregate differential
  tests fail without it.
- The `DebugNoScope` no-op survives. Its producer is the inliner, its shape is valid, and the
  emitter owns the absence of debug output.
- Literal-hash emission survives. It uses the already-validated literal and the stable byte-hash
  source of truth used by established emitters.
- No provider callback, compatibility fallback, fixture-name condition, arbitrary operand walk,
  syntax reconstruction, or downstream repair was added.

The next slice should choose among the four tied eight-workload healthy-MVP clusters by the number
and importance of representative kernels their canonical invariant unlocks, not by fixture order.
