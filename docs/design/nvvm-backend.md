# Direct NVVM Backend

## Purpose

Slang currently produces PTX by emitting CUDA C++ and compiling that source with NVRTC. This
design adds an experimental path that lowers linked Slang IR directly to NVVM IR and asks libNVVM
to produce PTX. The established NVRTC path remains available throughout development and remains
the default until the direct path meets explicit correctness, compatibility, compile-time, and
generated-code-quality gates.

The direct path is intended to remove a source-language round trip, preserve useful IR structure
for NVIDIA's optimizer, reduce compilation time, and avoid CUDA-C++ transformations that degrade
generated code. Each of those benefits is a hypothesis to measure rather than an assumption built
into the acceptance criteria.

## Agreed Initial Support Contract

The first implementation uses these boundaries:

- CUDA Toolkit 12.0 or newer;
- NVVM IR 2.0;
- the legacy typed-pointer NVVM IR dialect specified against LLVM 7.0.1, which has the broadest
  architecture reach;
- 64-bit NVPTX only, using the `nvptx64-nvidia-cuda` target triple;
- compute kernels first;
- textual NVVM IR only for bootstrap tests and diagnostics;
- a validated LLVM 14 typed-pointer bitcode-writer path as a production-readiness gate;
- an opt-in direct backend, with NVRTC remaining the default; and
- local validation on the current development machine until a CUDA/toolkit CI matrix is defined.

OptiX, source-level debug metadata, relocatable device code, device LTO, dynamic parallelism, and
device syscalls are separate later tracks. Resources, atomics, wave operations, advanced numeric
types, cooperative operations, and autodiff enter only after the basic compute ABI is stable.

## Pipeline Shape

The two PTX routes share the front end, Slang IR linking, and target-independent optimization, then
diverge at a named CUDA emission method.

```text
                                            CUDA source legalization
                                         -> CUDA C++ emitter -> NVRTC -----+
Slang source -> semantic checking                                               |
             -> linked Slang IR ---------+                                      +-> PTX
                                         |                                      |
                                         +-> NVVM legalization                  |
                                            -> NVVM IR emitter -> libNVVM ------+
                                                                  + libdevice
```

The final public output target stays `SLANG_PTX`. The proposed target option follows the existing
CPU and SPIR-V emission-method pattern:

```cpp
enum SlangEmitCUDAMethod
{
    SLANG_EMIT_CUDA_DEFAULT,
    SLANG_EMIT_CUDA_VIA_NVRTC,
    SLANG_EMIT_CUDA_VIA_NVVM,
};
```

The exact command-line spelling will be finalized with the routing slice. The intended form is
`-emit-cuda-via-nvrtc` and `-emit-cuda-via-nvvm`.

## Current Slang Pipeline

For an ordinary PTX request, `_getDefaultSourceForTarget` maps `CodeGenTarget::PTX` to
`CodeGenTarget::CUDASource` in
[`slang-code-gen.cpp`](../../source/slang/slang-code-gen.cpp#L253). The global session installs the
`CUDASource -> PTX` NVRTC transition in
[`slang-global-session.cpp`](../../source/slang/slang-global-session.cpp#L169).
`emitWithDownstreamForEntryPoints` therefore runs the linked IR pipeline and emitter as
`CUDASource`, then passes the resulting CUDA C++ artifact to NVRTC.

This detail matters because several switches in `linkAndOptimizeIR` test `CUDASource` directly.
Some preserve CUDA semantics, while others only prepare code for the CUDA C++ type system, prelude,
or NVRTC. The direct backend must audit every such branch. Running the existing pipeline as `PTX`
or reusing every `CUDASource` branch would both be incorrect shortcuts.

The existing LLVM emitter is also not a drop-in NVVM emitter. It is currently CPU-oriented, uses
LLVM opaque pointers, produces CPU workgroup dispatch helpers, and exposes no builder operations
for NVVM address spaces, kernel annotations, or GPU intrinsics. Useful scalar, type, and control-
flow concepts can be factored and shared later, but NVVM legalization and representation need a
target-specific owner.

## Compiler Boundaries

### CUDA emission method

The emission method selects the representation used after shared Slang IR work. Named predicates
should distinguish CUDA-family semantics from representation-specific behavior, for example:

- CUDA-family behavior required by both routes;
- CUDA-C++ source behavior required only by the NVRTC route; and
- NVVM legalization required only by the direct route.

This avoids growing scattered tests for `CodeGenTarget::CUDASource`, `CodeGenTarget::PTX`, and a
future intermediate target that accidentally encode different meanings of "CUDA."

### NVVM legalization and emitter

The NVVM layer owns:

- the legal subset of linked Slang IR accepted by the emitter;
- entry-point and device-function ABI lowering;
- LLVM/NVVM types, attributes, linkage, and symbol rules;
- generic, global, shared, constant, and local address spaces;
- kernel and global annotations;
- NVVM intrinsics and supported LLVM atomic representations;
- libdevice call selection; and
- rejection of unsupported operations before malformed NVVM IR reaches libNVVM.

libNVVM verification remains mandatory even when a Slang-side legality check passes.

### libNVVM downstream compiler

The first implementation adds `SLANG_PASS_THROUGH_NVVM` and a dynamically loaded
`NVVMDownstreamCompiler`, following the lifetime and artifact conventions used by the NVRTC
downstream compiler. It consumes one exact `Assembly + LLVMIR + Kernel` text artifact or
`ObjectCode + LLVMIR + Kernel` bitcode artifact, verifies it, compiles it, and returns a PTX
artifact with associated diagnostics. The kind is the representation contract: the compiler does
not sniff bitcode magic, append a text terminator, or otherwise rewrite the input bytes.

The compiler must query `nvvmVersion` and `nvvmIRVersion`. It resolves `nvvmLLVMVersion` as an
optional function because older CUDA 12 toolkits do not export it. The implementation must not
infer the NVVM IR or LLVM dialect only from a toolkit directory name.

`DownstreamCompilerDesc::version` contains the numeric NVVM version used by the public query API.
The compiler's fuller version string contains the NVVM and NVVM IR/debug tuples plus the loaded
library timestamp, because different patch builds can report the same numeric version. The
architecture-dependent result of `nvvmLLVMVersion` is combined with the selected target only by
later cache/routing work; it is not an instance-wide version.

### Intermediate artifact visibility

The bootstrap path uses an internal artifact equivalent to LLVM assembly for a kernel target. It
does not initially add a public `SLANG_NVVM_IR` compile target or overload the CPU-oriented
`SLANG_SHADER_LLVM_IR` target. Public exposure can be reconsidered after the emitter and dialect
contract stabilize.

Concretely, the bootstrap input descriptor is `Assembly + LLVMIR + Kernel`; the PTX output created
for `SLANG_PTX` is `ObjectCode + PTX + Kernel`. The initial compiler implements
`IDownstreamCompiler::compile` only. The generic `convert` entry point has no target architecture or
policy options, so advertising conversion would create an incomplete second contract.

## Public NVVM Contract

NVVM IR is a restricted form of LLVM IR. A module that is valid LLVM IR can still be illegal NVVM
IR. The implementation follows the public NVVM IR specification and libNVVM API; CICC internals are
diagnostic evidence only.

### Module envelope

The initial emitter uses the only supported 64-bit data layout:

```llvm
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-i128:128:128-f32:32:32-f64:64:64-v16:16:16-v32:32:32-v64:64:64-v128:128:128-n16:32:64"
target triple = "nvptx64-nvidia-cuda"
```

An NVVM IR 2.0 module declares its version and marks each kernel explicitly:

```llvm
!nvvmir.version = !{!0}
!nvvm.annotations = !{!1}
!0 = !{i32 2, i32 0}
!1 = !{void ()* @kernel, !"kernel", i32 1}
```

Calling-convention markings alone do not make a function an NVVM kernel.

### Address spaces

The initial mapping is:

| Address space | Meaning |
| --- | --- |
| 0 | generic pointers and code |
| 1 | global memory |
| 3 | shared memory |
| 4 | constant memory |
| 5 | local memory |

Address-space conversions use `addrspacecast`. Integer round trips are not a substitute for
preserving pointer provenance and aliasing information.

### API lifecycle

For each compile, the downstream compiler:

1. creates a fresh `nvvmProgram`;
2. adds the generated user module normally;
3. optionally adds toolkit-matched libdevice bitcode lazily;
4. verifies the program with the intended compile options;
5. records the complete verifier log;
6. compiles the program with the same policy options;
7. records the complete compiler log and retrieves the PTX; and
8. destroys the program through an RAII scope on every path.

Compile and log result sizes include the trailing NUL. An error path still returns an artifact with
associated diagnostics so the existing caller can report the failure consistently. A verifier or
compiler rejection follows the existing in-process downstream convention: mark the diagnostics as
failed, attach an error, require an error diagnostic, and return `SLANG_OK` with the PTX-desc
artifact. Interface or operational failures may return a failing `SlangResult` but still return the
artifact. When libNVVM supplies an empty program log, the diagnostic falls back to
`nvvmGetErrorString`. The PTX artifact excludes the API's trailing NUL; a missing terminator or an
otherwise empty payload is treated as an invalid vendor result rather than exposed as successful
PTX.

### Compile options

The direct route always passes an explicit virtual architecture, such as `-arch=compute_75`.
Toolkit defaults are not part of Slang's contract. The initial option mapping also covers:

- optimization as `-opt=0` or `-opt=3`;
- flush-to-zero as `-ftz=0|1`;
- precise single-precision square root as `-prec-sqrt=0|1`;
- precise single-precision division as `-prec-div=0|1`; and
- FMA contraction as `-fma=0|1`.

`FloatingPointMode::Fast` selects approximate division/square-root operations and enables FMA
contraction. `FloatingPointMode::Precise` selects the precise operations and disables contraction;
the default mode leaves all three libNVVM defaults unchanged. Full debug information maps to `-g`
only for an unoptimized compile, as required by libNVVM. Minimal and standard line information are
metadata-driven and add no libNVVM option.

NVRTC's spellings and aggregate fast-math option differ, so differential tests must set both paths
explicitly instead of relying on either compiler's defaults.

## Discovery and Toolkit Coherency

libNVVM and libdevice must come from one selected CUDA toolkit root. The expected layout is:

```text
<toolkit>/nvvm/include/nvvm.h
<toolkit>/nvvm/bin/<libNVVM DLL>             # Windows
<toolkit>/nvvm/lib64/<libNVVM shared object> # Linux
<toolkit>/nvvm/libdevice/libdevice.10.bc
```

An explicit `-nvvm-path` wins. A decorated file path is normalized to the unadorned name/path
required by `ISlangSharedLibraryLoader`, while its original resolved identity remains available for
diagnostics. The locator then attempts the logical library name through Slang's
shared-library loader before probing filesystem layouts; this supports injected test loaders and
platform loader installations. Filesystem discovery considers the Slang module location,
the component-specific `LIBNVVM_HOME`, `CUDA_PATH`, `CUDA_HOME`, and CUDA toolkit roots represented
on `PATH` in deterministic order. Within one directory, versioned library names are ranked by
numeric suffix rather than lexical filename order. Windows discovery covers both `nvvm\bin` and
the CUDA 13 `nvvm\bin\x64` layout. The first implementation keeps this locator self-contained;
extracting a CUDA-toolkit-root helper shared with NVRTC remains worthwhile only when focused tests
can prove NVRTC's existing selection order is unchanged.

After a logical/system load, the implementation derives the actual library filename and toolkit
root from a resolved symbol when the platform exposes them. A compiler may remain usable for a
libdevice-free module when that root is unknown, but later libdevice linking must resolve a
coherent root rather than combine components speculatively. Discovery must diagnose "NVRTC is
installed but the libNVVM component is missing" separately from "no CUDA toolkit was found."
Library basenames and package composition are versioned inputs and must be probed rather than
frozen into one permanent filename.

## Dialect and Bitcode Gate

The bootstrap compiler accepts textual LLVM-7-dialect NVVM IR because it is readable, easy to
minimize, and supported by the installed library. NVIDIA deprecates textual input, so that is not
the production representation.

The Slice 2 prototype established external-writer compatibility and the consumer artifact boundary
on the local CUDA 12.2 toolkit:

- llvmlite 0.42.0's LLVM 14.0.6 writer serialized the minimal typed-pointer NVVM IR 2.0 kernel to
  1,668 bytes of bitcode. libNVVM verified and compiled it for `compute_75`, and `ptxas` accepted
  the resulting PTX. The checked-in fixture records the source, exact regeneration procedure, and
  SHA-256. NVIDIA's LLVM 7-14 sample guidance concerns a text-IR workflow, so the fixture is the
  empirical evidence for LLVM 14 bitcode rather than an NVIDIA compatibility guarantee.

The existing `slang-llvm` module is not the producer for the pre-Blackwell backend. It uses LLVM
21, exposes opaque pointers, and has no named-metadata or address-space builder interface. Its stock
configuration does not request NVPTX or expose BitWriter/bitcode output. Adding only a
`generateBitcode` method would still leave the wrong dialect and an insufficient builder API.

Slice 3a implements that direction as a separate optional NVVM builder module pinned to LLVM 14,
the newest release in NVIDIA's documented LLVM 7-14 sample range and the version exercised by the
accepted fixture. It owns typed-pointer construction, NVVM metadata, and bitcode serialization
independently from the LLVM 21 CPU module. Only the resulting bytes cross into compiler-core. This
is a candidate production boundary, not a production-ready component; its packaging, CI matrix,
and nontrivial IR coverage remain to be proven.

Coexistence of the two LLVM versions is a hard module invariant. LLVM 14 must be configured and
built separately from LLVM 21, statically link only the LLVM components needed by the NVVM writer,
hide or exclude raw LLVM symbols, and export only a versioned Slang ABI. The optional module must
not introduce a process-visible dynamic `libLLVM` dependency that can collide with `slang-llvm` or
other LLVM users. The first builder slice must validate those properties before adding broad IR
lowering.

### Slice 3a builder boundary

`source/slang-llvm-nvvm` is an independent CMake project rather than a subdirectory of Slang's
normal configure. It requires exactly LLVM 14.0.6 through `SLANG_NVVM_LLVM_DIR`, rejects LLVM built
with C++ exceptions, rejects linking the shared LLVM dylib, and requires the selected Core,
BitWriter, and Support targets to be static libraries. This keeps LLVM 14 and the normal LLVM 21
`slang-llvm` package out of the same CMake target namespace.

At the Slice 3a boundary the module exports one symbol, `slang_getNVVMBuilderAPI_V1`. That getter
returns a size- and version-checked function table from an LLVM-free header that compiles as C and
C++. The ABI uses fixed-width version/result/classification fields, native-size counts, opaque
module/type/value/block handles, pointer-plus-count strings, and caller-owned serialization
storage. No LLVM type, C++ object, allocator-owned buffer, or exception crosses the DLL boundary.
Handles must be live and originate from the module supplied to the call; destroying a module
invalidates all subordinate handles.

Version 1 deliberately contains only general operations needed to build the first structural
proof: create a module, obtain `void`, form a function type, declare a function, append/select a
block, emit `ret void`, add the NVVM kernel annotation, and serialize assembly or bitcode. Module
creation fixes the 64-bit NVPTX triple/data layout and NVVM IR 2.0 metadata. Serialization runs
LLVM's verifier first and uses a size-query/write protocol so insufficient storage is reported
without partially modifying the caller's buffer.

The host-side `NVVMIRBuilder` retains the loaded library, validates the exact LLVM 14.0.6/NVVM 2.0
typed-pointer contract, rejects successful calls that omit required handles, and copies serialized
bytes into a host-owned blob. It does not register another downstream compiler: generated bitcode
is wrapped as `ObjectCode + LLVMIR + Kernel` and passed to the existing
`NVVMDownstreamCompiler`. Focused tests look for the optional module beside the test executable or
in the directory named by `SLANG_NVVM_BUILDER_PATH`; an explicit but broken directory is a failure,
not a skip.

The local Windows proof uses upstream tag `llvmorg-14.0.6` at commit
`f28c006a5895fc0e329fe15fead81e37457cb1d1`. The Slice 3a Release module has only the V1 getter in
its PE export table and no LLVM DLL dependency. In one test process, the normal LLVM 21.1 module is
loaded first, the LLVM 14 module constructs caller-named empty kernels, CUDA 12.2 libNVVM compiles
their bitcode to PTX, and CUDA 12.2 `ptxas -arch=sm_75` accepts that PTX. The reverse fresh-process
load order was left for Slice 3b; non-Windows binary inspection remains later CI/packaging work.

### Slice 3b diagnostic and ABI evolution boundary

Slice 3b freezes `SlangNVVMBuilderAPI_V1` and its getter byte-for-byte. V2 composes one complete V1
table with a required `serializeModuleWithDiagnostics` operation. Unlike V1, V2 is append-only and
size-negotiated: the caller supplies its structure capacity, the provider reports its complete
structure size, and the caller requires only the prefix through the last field it uses. A larger
provider table is therefore compatible, while changing an existing field's signature or semantics
still requires a new ABI version. The Slice 3b minimum-prefix constant remains frozen when fields
are appended.

The host probes V2 first. It falls back to V1 only when the V2 symbol is absent; finding a V2
getter and then accepting a malformed table would hide a broken deployment. V1-only providers
retain the empty-kernel capability, and the host reports that serialization diagnostics are not
available. A new provider continues to export V1 for older hosts and exports V2 for the diagnostic
path. These versioned getters remain the complete export allowlist; no LLVM symbol becomes visible
across the module boundary.

`serializeModuleWithDiagnostics` returns three logically separate outputs from one operation:
serialized bytes, LLVM verifier bytes, and a fixed-width verification status. Its `SlangResult`
reports whether argument validation and the caller-owned buffer transport succeeded. The status
classifies the LLVM work as `NOT_RUN`, `VALID`, or `INVALID`. `NOT_RUN` accompanies a transport
failure before verification. `VALID` has non-empty serialized output and may include diagnostic
bytes, allowing a future provider to report verifier warnings without another ABI revision.
`INVALID` is a successful transport transaction with no serialized output and a non-empty captured
LLVM verifier diagnostic. If LLVM reports invalidity without emitting text, the provider supplies
a stable fallback message. The host copies those bytes before mapping the status to a compilation
failure.

Both byte outputs use one atomic query/write protocol. A query passes null destinations and zero
capacities and receives exact byte counts. Diagnostic counts exclude a trailing NUL, and arbitrary
bytes, including embedded NULs, are preserved. Before a write copies either output, the provider
checks both capacities. An insufficient buffer reports both required sizes and the verification
status, returns `SLANG_E_BUFFER_TOO_SMALL`, and modifies neither destination. Query and write calls
run independently, and the host rejects changes in sizes or status between them. Callers provide
non-overlapping destination ranges and output-metadata storage. No provider-owned pointer,
allocator, C++ object, or mutable global "last error" crosses the ABI.

The two statically linked LLVM versions are exercised in both orders in fully isolated test-server
processes. The LLVM-first order queries LLVM 21.1, uses the LLVM 14 NVVM builder, and queries LLVM
21.1 again. The NVVM-first order uses LLVM 14, loads and queries LLVM 21.1, then uses LLVM 14 again.
This isolation is necessary because the ordinary test coordinator probes pass-through compilers
before it dispatches a selected unit test.

Slice 3b does not expand the generated IR. Integer or pointer types, address spaces, parameters,
constants, memory operations, and scalar reference kernels remain the Slice 4 boundary.

Modern-dialect support is a later route selected from the target architecture and the optional
`nvvmLLVMVersion` query. It is not a prerequisite for legacy-dialect compute support.

## CUDA Pass Ownership Audit

Before the first Slang-to-NVVM emitter lands, each current CUDA-specific behavior must be placed in
one of four groups: shared CUDA semantics, CUDA-C++ representation, NVVM representation, or obsolete
after the split. Important initial audit items include:

| Current behavior | Audit question |
| --- | --- |
| OptiX entry-point uniform collection | Defer entirely, or identify a compute semantic hidden by the OptiX-specific branch? |
| CUDA entry-point preservation and builtin lowering | Which invariants are common to both routes? |
| Global-constant inlining | Is this only an NVRTC dynamic-initialization workaround? |
| CUDA varying-parameter legalization | Which part defines the CUDA launch ABI and which part only emits C++ parameters? |
| Parameter-copy and const-reference transforms | Can NVVM preserve values/SSA instead of reconstructing C++ reference semantics? |
| Phi elimination | Keep SSA for NVVM unless a measured libNVVM constraint requires otherwise. |
| Resource legalization disabled for CUDA source | Define the concrete NVVM resource representation instead of relying on prelude templates. |

Every retained special case needs a concrete producer, canonical input shape, downstream consumer,
and test proving that this layer owns it.

## Validation Strategy

PTX text equality is not a correctness requirement. The established and direct paths may choose
different legal instructions, names, register layouts, and PTX versions. Validation is layered:

1. `nvvmVerifyProgram` succeeds and its log is retained;
2. `ptxas` accepts the PTX for the selected architecture;
3. an ABI manifest agrees on entries, parameters, alignments, globals, address spaces, and launch
   directives;
4. both paths produce the same runtime results for the same inputs;
5. focused tests check semantically important PTX instruction families;
6. `ptxas -v` records registers, spills, stack/local/shared memory, and code size; and
7. benchmarks separate Slang lowering time, downstream compilation time, total time, and kernel
   runtime.

The capability ledger will classify tests by feature rather than treating every test under
`tests/cuda` as a direct-backend test. CUDA-source syntax, prelude, macro, and C++ emission tests need
semantic NVVM counterparts.

## Test Buckets and Capability Ledger

Tests advance by the first backend capability they require, not merely by directory or historical
test name. The initial buckets are:

| Bucket | Capability boundary | Representative contents |
| --- | --- | --- |
| 0 | External compiler contract | handwritten NVVM IR, loader failures, verifier logs, `ptxas` acceptance |
| 1 | Minimal compute ABI | empty kernel, scalar value/pointer parameters, launch shape, entry naming |
| 2 | Scalar program structure | arithmetic, comparisons, branches, loops, SSA phis, direct calls |
| 3 | Types and memory | vectors, matrices, aggregates, layout, generic/global/shared/local address spaces |
| 4 | Core CUDA execution | thread/block IDs, barriers, atomics, shared memory, memory ordering |
| 5 | Numeric library policy | half/bfloat/fp8, transcendental math, libdevice, fast/precise/denormal modes |
| 6 | Resource ABI | buffers, textures, samplers, surface operations, bindless/resource handles |
| 7 | Slang language lowering | generics, interfaces, witness tables, specialization, autodiff |
| 8 | Advanced NVIDIA paths | waves, cooperative features, OptiX, debug metadata, RDC, LTO, dynamic parallelism |

Source-emission tests whose contract is specifically CUDA C++ spelling, macro expansion, header
shape, or prelude text stay assigned to NVRTC. Add a semantic counterpart before using such a test
as evidence for NVVM.

Once the experimental routing slice exists, a checked-in capability ledger should record at least:

- test path and feature bucket;
- required target/profile and runtime hardware, if any;
- NVRTC result and NVVM result;
- first failing phase (`legalize`, `emit`, `verify`, `compile`, `ptxas`, or `runtime`);
- a stable diagnostic category or unsupported Slang IR instruction;
- ABI/runtime comparison status; and
- downstream time, PTX size, `ptxas` resource summary, and kernel time when measured.

Work proceeds within the lowest incomplete bucket. A test moves to a higher bucket only when its
first failure is genuinely a later capability; it is not reclassified simply to make the current
bucket appear complete.

## Development Slices

The program advances through bounded slices:

1. architecture contract and a dynamically loaded libNVVM compiler that handles handwritten IR;
2. LLVM-7-compatible bitcode compatibility and artifact-boundary feasibility;
3. a separately built/loaded LLVM 14 NVVM builder with a versioned ABI and empty-kernel bitcode;
4. scalar and pointer reference kernels with NVRTC-versus-libNVVM differential evidence;
5. CUDA emission-method selection and experimental PTX routing through the registered compiler;
6. minimal Slang IR compute lowering;
7. scalar control flow and the kernel ABI;
8. address spaces, aggregates, and shared memory;
9. libdevice and floating-point policy;
10. atomics and wave operations;
11. resources and optimization-quality work; and
12. advanced capabilities and production-readiness evaluation.

Slice 3b hardens the builder boundary between items 3 and 4 with versioned verifier diagnostics and
the reverse LLVM load-order proof; it deliberately adds none of item 4's scalar or pointer surface.

Each slice has its own local ExecPlan and leaves the NVRTC path usable.

## Confirmed Local Evidence

On 2026-08-25, a manual process-local probe loaded:

```text
C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.2\nvvm\bin\nvvm64_40_0.dll
```

The library reported NVVM 2.0 and NVVM IR 2.0 with debug metadata 3.1. It rejected an IR 1.11
module with `NVVM_ERROR_IR_VERSION_MISMATCH`, then verified an IR 2.0 empty-kernel module and
compiled it for `compute_75`. The result was PTX 8.2 containing:

```text
.version 8.2
.target sm_75
.visible .entry testEmpty()
```

This proves API availability and the minimum textual-IR lifecycle on this machine. It does not
prove bitcode compatibility, Slang lowering, runtime correctness, or performance.

The first downstream-compiler slice subsequently reproduced that result through Slang's artifact
interface. Sixteen focused tests cover injected discovery, required and optional ABI symbols,
numeric candidate ranking, decorated-path normalization, shared-library and program lifetimes,
input-contract rejection, result classification, option translation, multi-phase diagnostics,
malformed vendor results, real libNVVM compilation, and offline `ptxas` assembly. On the same
machine all sixteen passed, `-nvvm-version` reported 2.0, and CUDA 12.2 `ptxas` 12.2.140 accepted
the generated `sm_75` PTX. Ordinary PTX compilation remained routed through NVRTC.

The bitcode slice then added the exact binary artifact contract and two focused tests. The full
NVVM prefix passed 18/18. Its embedded 1,668-byte LLVM 14.0.6 fixture was forwarded byte-for-byte,
verified and compiled by the same CUDA 12.2 libNVVM, and the resulting `sm_75` PTX was assembled by
`ptxas`. The existing LLVM 21 module was ruled out as the pre-Blackwell producer; a separate pinned
LLVM 14 NVVM builder prototype is the next prerequisite.

## Settled and Open Decisions

Settled decisions are the support contract at the top of this document, the parallel backend
shape, the continued NVRTC default, the binary artifact shape, the separate pinned LLVM 14 builder,
and the frozen-V1/append-only-V2 diagnostic ABI described above.

The following remain open until their named slice supplies evidence:

- packaging and update policy for the optional LLVM 14 NVVM builder module;
- the CUDA toolkit and GPU CI matrix;
- the final public spelling and API exposure of the CUDA emission method;
- whether NVVM IR should become a public compile target;
- the exact entry-point/global-parameter ABI beyond the first pointer/scalar kernels;
- the scope of source-level debugging; and
- production thresholds for compile time, resource use, and runtime performance.

## Authoritative References

- [NVVM IR specification](https://docs.nvidia.com/cuda/nvvm-ir-spec/index.html)
- [libNVVM API](https://docs.nvidia.com/cuda/libnvvm-api/index.html)
- [NVIDIA libNVVM samples](https://github.com/NVIDIA/cuda-samples/tree/master/Samples/7_libNVVM)
- [libdevice basic usage](https://docs.nvidia.com/cuda/libdevice-users-guide/basic-usage.html)
- [NVRTC documentation](https://docs.nvidia.com/cuda/nvrtc/)

The [CICC reverse-engineering notes](https://github.com/GrigoryEvko/crucible-notes/tree/main/cicc)
may suggest focused experiments, but their own project disclaimer identifies them as static,
AI-generated best-guess reconstructions. They do not override the public specifications or runtime
probes.
