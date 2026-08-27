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
- textual NVVM IR only as an explicitly negotiated compatibility path or for diagnostics;
- exact-version typed-pointer writers, including the isolated LLVM 7.0.1 native-bitcode experiment
  and the LLVM 14.0.6 construction path, as production-readiness gates;
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

The final public output target stays `SLANG_PTX`. The target option follows the existing CPU and
SPIR-V emission-method pattern:

```cpp
enum SlangEmitCUDAMethod
{
    SLANG_EMIT_CUDA_DEFAULT,
    SLANG_EMIT_CUDA_VIA_NVRTC,
    SLANG_EMIT_CUDA_VIA_NVVM,
};
```

The finalized command-line spellings are `-emit-cuda-via-nvrtc` and
`-emit-cuda-via-nvvm`. They are target-scoped selectors for one canonical option; if both occur
for the same target, the last one wins. An absent option means `SLANG_EMIT_CUDA_DEFAULT`.

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

1. validates the source and compile policy and, when the compatible options explicitly require the
   CUDA device library, reads the toolkit-matched `libdevice.10.bc`;
2. creates a fresh `nvvmProgram`;
3. adds the generated user module normally;
4. when requested, adds libdevice lazily (or normally only when the optional lazy-add API is
   absent);
5. verifies the program with the intended compile options;
6. records the complete verifier log;
7. compiles the program with the same policy options;
8. records the complete compiler log and retrieves the PTX; and
9. destroys the program through an RAII scope on every path.

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

Slice 18 freezes the floating-point controls as two independent matrices:

| Slang floating-point mode | libNVVM options |
| --- | --- |
| Default | omit `-prec-div`, `-prec-sqrt`, and `-fma` |
| Precise | `-prec-div=1`, `-prec-sqrt=1`, and `-fma=0` |
| Fast | `-prec-div=0`, `-prec-sqrt=0`, and `-fma=1` |

| fp32 denormal mode | libNVVM option |
| --- | --- |
| Any | omit `-ftz` |
| Preserve | `-ftz=0` |
| FlushToZero | `-ftz=1` |

The two selected rows compose, and the exact resulting vector is supplied unchanged to both
`nvvmVerifyProgram` and `nvvmCompileProgram`. Non-default fp16 or fp64 denormal modes are rejected
before program creation because these controls are specifically fp32 policy. Compiler-specific
arguments may not duplicate or override the managed `-ftz`, `-prec-div`, `-prec-sqrt`, or `-fma`
families; the typed fields are the sole source of truth. Unrecognized floating-point-mode or fp32
denormal-mode enum values are also rejected before program creation.

NVRTC's spellings and aggregate fast-math option differ, so differential tests must set both paths
explicitly instead of relying on either compiler's defaults. Slice 18 does not change NVRTC's
aggregate fast-math behavior or claim option parity between the two downstream compilers.

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

Slice 18 preserves the exact successful filesystem candidate path through compiler construction.
Only a canonical `<root>/nvvm/bin[/x64]` or `<root>/nvvm/lib[64]` library path yields a selected
toolkit root; symbol-path derivation remains the fallback for logical/system loading. A requested
device library is read only from `<selected-root>/nvvm/libdevice/libdevice.10.bc`. Failure to derive
that root or read that exact opaque bitcode file is reported before program creation. The compiler
does not retry `CUDA_PATH`, `CUDA_HOME`, `LIBNVVM_HOME`, or `PATH`, and does not silently combine a
selected libNVVM with another toolkit's libdevice. When the coherent file is present, its timestamp
joins the downstream compiler identity; rootless compilers retain their libdevice-free identity.
The explicit demand controls compilation: a zero value never requires, reads, or adds libdevice.
Version-identity lookup is separate and may stat the exact coherent path when a selected toolkit
root is known.

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

Slice 6 makes the builder session-owned and lazily loaded so shader hashing and code generation use
the same retained provider. Changing the session's shared-library loader clears that cached
instance. The direct Slang route requires the V2 verified-serialization prefix so malformed
generated IR always carries LLVM verifier text. A V1-only provider still supports the standalone
builder proof, but is incompatible with direct Slang lowering and reaches E52016.

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

### Slice 4 scalar-memory builder and differential boundary

Slice 4 appends one coherent capability block to V2 without changing V1 or the frozen Slice 3b V2
prefix. The block creates signless integer types and typed pointers in an explicit NVVM address
space, obtains a declared function parameter, and emits aligned non-volatile loads and stores. A
second minimum-size constant names the complete block. An old provider reporting exactly the Slice
3b prefix remains valid but does not advertise scalar operations. A provider reporting the new
minimum must supply every operation; a byte size strictly between the two published minima or a
missing function is malformed. Future/larger providers remain compatible, and the host retains
the lesser of the provider-reported size and its local table capacity instead of inflating an old
table to the local `sizeof`.

Each module owns a unique LLVM context, which is the canonical ownership boundary for returned
types and values. Pointer types carry their NVVM address space directly; this slice does not round
trip them through integers or insert casts. Loads and stores require a current module-owned,
unterminated block, a typed pointer from the same context, and a nonzero power-of-two byte
alignment. Stores additionally require an exact pointee/value type match and reject address space
4 because NVVM constant memory is read-only. The provider validates the complete shape before
inserting an instruction. Host wrappers clear outputs before dispatch and also clear a handle if a
provider writes one and then reports failure, so a failed call never exposes stale provider state.

The first two reference kernels are the exact CUDA shapes:

```cpp
extern "C" __global__ void writeScalar(int* destination, int value)
{
    *destination = value;
}

extern "C" __global__ void copyScalar(int* destination, const int* source)
{
    *destination = *source;
}
```

The builder represents both pointer parameters as `i32 addrspace(1)*`. Global address space 1 is
intentional: the source contract identifies device-global storage, and LLVM 14 NVPTX lowers that
canonical type directly to global load/store operations. Generic address space 0 would introduce
a conversion and is reserved for a later lowering case that actually produces generic pointers.
The kernels need no constants, GEP, casts, arithmetic, control flow, or synthesized LLVM text.

The differential test compiles the exact CUDA source with NVRTC and the builder-produced bitcode
with libNVVM for `compute_75`. It compares named-entry parameter order and widths—`[64, 32]` for
`writeScalar` and `[64, 64]` for `copyScalar`—and requires entry-scoped 32-bit global store, or
global load plus store, respectively. It deliberately ignores PTX spelling, parameter names,
register allocation, whitespace, and tool-selected PTX version. CUDA-toolkit `ptxas -arch=sm_75`
acceptance is the final static syntax and target gate for both routes.

This evidence does not change routing: ordinary PTX requests still use NVRTC, and no Slang IR
lowering exists at this boundary. It also does not prove strict toolkit identity. Default NVRTC
discovery may prefer a library beside the executable while libNVVM and `ptxas` are rooted at the
configured CUDA toolkit. Successful assembly proves that the selected tools interoperate on the
tested machine; a shared toolkit-root selection contract remains separate discovery work.

### Slice 5 target-option and routing boundary

Slice 5 freezes `SlangEmitCUDAMethod` as `DEFAULT = 0`, `VIA_NVRTC = 1`, and `VIA_NVVM = 2`.
`CompilerOptionName::EmitCUDAMethod`, `EmitCUDAViaNVRTC`, and `EmitCUDAViaNVVM` are appended as
numeric values 158, 159, and 160. The two selector names are command-line vocabulary only: parsing
stores the canonical `EmitCUDAMethod`, and command-line reconstruction maps an explicit canonical
value back to its selector. CLI selections belong to their target. API clients may also supply the
canonical option through `linkWithOptions`; the `TargetProgram` effective option set applies that
component/link-time override consistently to codegen and shader hashing. Only PTX consumes it.

PTX dispatch now resolves four cases at one boundary:

1. Explicit pass-through retains precedence because it already defines the input representation.
2. The absent/default method follows the session's existing `CUDASource -> PTX` transition.
3. Explicit NVRTC selects the registered NVRTC compiler without mutating that session transition.
4. Explicit NVVM enters a dedicated route and never sends CUDA source to the LLVM-bitcode
   consumer or falls back to NVRTC.

`getDownstreamCompilerRequiredForPTXTarget` is the single method-to-compiler mapping used by PTX
dispatch, the effective-option cache-hash path, and focused tests. Consequently, the compiler
prelude and version identity hashed by `Linkage::buildHash` describe the compiler selected for that
target program, including `linkWithOptions`. An invalid API-provided method maps to no compiler and
is diagnosed as E52015 instead of silently choosing a route. A malformed non-integer option shape
violates the public option contract and is release-asserted at the effective-option accessor.

True `-pass-through` selection remains state on the legacy `EndToEndCompileRequest`, not on a
`ComponentType`, and therefore does not participate in the component shader-hash API. The mixed
pass-through/method regression proves dispatch precedence; the component hash describes the
ordinary non-pass-through target-program route.

At the Slice 5 boundary the direct route intentionally stopped at E52014. Slice 6 replaces that
historical stop with the first canonical Slang-IR-to-NVVM producer.

### Slice 6 minimal linked-IR compute boundary

Consider this program:

```slang
[numthreads(1, 1, 1)]
void computeMain()
{}
```

The direct PTX route now runs the ordinary link-and-optimize pipeline without entering a
CUDA-source subcontext. The resulting selected entry point must be a defined compute function
returning `void`, with no parameters, exactly one parameterless block, and only
`IRReturn(IRVoidLit)`. Its `IREntryPointDecoration` remains the source of truth for the emitted
kernel name. Any other reachable instruction or semantic global is rejected with E52017; the
emitter neither repairs the IR nor substitutes an empty kernel.

After legality succeeds, the session-owned LLVM 14 builder creates an NVPTX64/NVVM 2.0 module,
declares the selected `void()` function, emits one `ret void`, marks it as a kernel, and verifies
and serializes LLVM bitcode through the V2 API. Builder discovery or ABI incompatibility reaches
E52016. A failed builder operation or verifier rejection reaches E52018, with verifier text
preserved, and no libNVVM program is created.

The resulting `ObjectCode + LLVMIR + Kernel` artifact enters the existing downstream continuation
with `SourceLanguage::LLVM` and `PassThroughMode::NVVM`. That continuation remains the single owner
of architecture and compilation options, diagnostics, timing, and artifact associations.
`cuda_sm_7_0` becomes `-arch=compute_70`, and successful compilation returns
`ObjectCode + PTX + Kernel`.

The shader hash includes both the registered libNVVM identity and the separately loaded builder
identity. Hash construction can discover the optional builder before code-generation legality is
checked, so the provider-independent unsupported-IR guarantee is specifically that no builder
module or libNVVM program is created, not that no library-load attempt occurs.

This slice deliberately excludes parameters, values, memory operations, calls, branches, loops,
builtins, resources, multiple entry points, and every non-compute stage. Slice 7 extends that
boundary with a deliberately small raw CUDA scalar ABI and acyclic control flow.

### Slice 7 raw CUDA scalar ABI and control-flow boundary

Consider these two raw CUDA kernels:

```slang
[CUDAKernel]
void writeScalar(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = value;
}

[CUDAKernel]
void chooseScalar(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    if (x < y)
        *destination = x + y;
    else
        *destination = x - y;
}
```

Slice 7 preserves the raw parameter order and accepts only signed `int` plus
`Ptr<int, Access::Read, AddressSpace::Device>` or
`Ptr<int, Access::ReadWrite, AddressSpace::Device>`. An `int` becomes an LLVM `i32`; both pointer
spellings become `i32 addrspace(1)*`, while the access qualifier remains a Slang-side legality
rule that rejects stores through the read-only spelling. Loads and stores are aligned to four
bytes. This is the ABI of an explicitly decorated `[CUDAKernel]`, not a general shader-entry ABI.
The Slice 6 zero-parameter `[shader("compute")]` case remains valid, but a conventional
parameterized compute entry is rejected rather than silently assigned the raw CUDA ABI.

The executable subset is `load`, `store`, signed `i32` `add` and `sub`, signed `i32` less-than,
`ifElse`, zero-argument unconditional branches, and `void` return. The emitter first declares all
blocks so forward branch targets exist, then emits each block body while maintaining a one-to-one
map from accepted Slang IR values and blocks to builder handles. The conditional reference kernel
stores its result in each arm and therefore needs no merge value or phi.

Legality is a complete preflight before builder-module or libNVVM-program creation. The selected
function must be the only selected entry point, have the exact raw parameter types above, and use
only values defined by accepted parameters or instructions that dominate each consumer. Blocks
must belong to that function, branch targets must be among those blocks, and every non-entry block
must have no parameters. The CUDA entry-point pruning pass still runs for the direct route; only
the exact selected entry point is retained afterward, so an unrelated `[CUDAKernel]` does not
become a second semantic global. This keeps selection in the linked-IR producer instead of teaching
the emitter to ignore an accidentally retained kernel.

The LLVM 14 provider repeats the ownership boundary at the ABI edge. Non-constant operands must
belong to the current module, function-local values must belong to the current function, and an
instruction must be available at the current insertion point. Branch blocks must also belong to
that function. Invalid or post-terminator operations are checked before mutation, so a rejected
call cannot leave partial LLVM IR behind. The host wrappers also clear output handles before and
after a failed provider call.

The control-flow operations are a second append-only V2 capability block after Slice 4's scalar
memory block: integer `add`/`sub`, signed less-than, unconditional branch, and conditional branch.
The original diagnostic and scalar-memory minimum-size constants remain frozen. A provider that
reports exactly the scalar-memory prefix remains valid and supports the two straight-line memory
kernels, but does not advertise control flow. A size inside the new block, or a complete size with
any required operation missing, is malformed. A complete or larger table is accepted, and the
builder identity records whether the control-flow prefix is present.

Executable constants, basic-block parameters and branch arguments (and therefore SSA phis), loop
lowerings, calls, complex/aggregate types, address spaces other than device-global pointer
parameters, builtins, resources, multiple selected entry points, and non-compute stages remain
outside this boundary. In particular, no general loop form is claimed merely because the builder
can create a backedge; ordinary loop-carried state requires the still-unsupported phi boundary.

### Slice 8 signed-i32 constants, SSA values, and finite loops

Consider this ordinary merge:

```slang
[CUDAKernel]
void selectValue(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    int selected;
    if (x < y)
        selected = x;
    else
        selected = y;
    *destination = selected;
}
```

The linked Slang IR already represents `selected` canonically: the merge block has one signed-i32
parameter, and each arm's unconditional edge carries its chosen value as argument zero. Slice 8
keeps that representation. It creates an LLVM `phi i32` for the block parameter and later attaches
`(x, trueBlock)` and `(y, falseBlock)` as incoming pairs. It does not reconstruct a local variable,
eliminate the phi, or infer values by walking unrelated operands.

The same rule handles canonical loop-carried values. For `for (int i = 0; i < limit; ++i)`, linked
IR uses an `IRLoop` entry edge carrying the initial `i` and sum, then an unconditional backedge
carrying their updated values. `IRLoop`'s target is the executable CFG successor; its break and
continue operands are structured metadata and are validated as same-function blocks but are not
emitted as extra LLVM edges. Both the entry edge and backedge pair their arguments with header
parameters by index.

Lowering therefore has four ordered phases. It creates all blocks, creates all destination-block
phis, emits ordinary bodies and terminators, then attaches phi incoming pairs after all backedge
values and predecessor terminators exist. Signed-i32 literals are identity-cached and materialized
on their first body or incoming-edge use because constants have no CFG lifecycle. The phase split is
a lifecycle constraint of SSA construction, not an alternative representation of the program.

Complete legality still runs before builder discovery. Only exact signed-i32 executable literals,
signed-i32 non-entry block parameters, and matching argument-bearing unconditional or loop edges
are added. Every parameterized block must have actual predecessors, and every predecessor must
supply the exact parameter count and types. A conditional edge cannot target a parameterized block
because `IRIfElse` carries no values. The validator also checks loop metadata ownership and rejects
branches to the function entry block.

The private V2 provider table gains one coherent append-only scalar-SSA block:

```text
getIntegerConstant(module, integerType, signedValue, outValue)
emitIntegerPhi(module, targetBlock, integerType, outValue)
addIntegerPhiIncoming(module, phi, value, predecessorBlock)
```

The first operation accepts only exactly representable signed values. Phi placement names its
destination block explicitly, so the provider inserts before the first non-phi instruction without
depending on ambient insertion state. Adding an incoming pair requires a complete function CFG,
the exact phi type and function, one real predecessor edge, no duplicate predecessor, and a value
that dominates the predecessor terminator. Every failure is checked before constant, instruction,
or incoming-list mutation. The Slice 3b, Slice 4, and Slice 7 minimum sizes remain frozen; a provider
inside the new prefix or missing any of its three functions is malformed, while an exact older
prefix retains all of its earlier capabilities.

This boundary accepts executable `0` and `1`, merge phis, and finite loops with signed-i32
loop-carried values. It does not claim termination analysis or add multiplication, calls, non-void
helper signatures and returns, other scalar types, pointer/aggregate phis, richer terminators, or
additional address spaces. Direct calls and their helper ABI are the Slice 9 boundary.

### Slice 9 direct signed-i32 helper functions

Consider this transitive helper graph:

```slang
int increment(int value)
{
    return value + 1;
}

int incrementTwice(int value)
{
    return increment(increment(value));
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = increment(value) + incrementTwice(value);
}
```

The final linked IR retains three module-owned function definitions. Each call names its exact
`IRFunc` as operand zero, followed by signed-i32 arguments, and each helper ends in an `IRReturn`
whose operand is the canonical signed-i32 SSA result. An unrelated helper with multiplication is
removed by ordinary linking and dead-code elimination before direct-NVVM preflight. Slice 9 follows
those exact callee operands from the one selected kernel; it does not make every module function an
entry point, look helpers up by source name, or reconstruct a function from syntax.

The complete direct-call closure is validated before builder discovery. Every helper must be a
defined, module-global, non-entry `IRFunc` with a signed-i32 result and only signed-i32 parameters.
Calls must have the exact result, argument count, and argument types, and each argument must be
available and dominate its call. Function values may occur only as operand zero of a direct call.
An active/completed traversal rejects cycles defensively, while the retained-global scan ensures no
semantic function is silently dropped. Each function then passes the same independent block, CFG,
SSA, phi, and dominance validation established in Slice 8. Entry returns remain void; helper
returns must carry the declared signed-i32 value.

Emission declares and maps every function and its parameters before creating any body. For each
function it then creates all blocks and phi placeholders, emits bodies in the same reachable-RPO
order used by preflight, and attaches phi incoming pairs after that function's CFG is complete.
Calls refer to the predeclared callee handle, helpers use their canonical mangled linkage names, and
only the selected `computeMain` function receives the NVVM kernel annotation. This ordering permits
forward and transitive calls without making physical module order semantic.

The private V2 provider table gains one coherent append-only scalar-function block:

```text
emitIntegerCall(module, callee, arguments, argumentCount, outValue)
emitIntegerReturn(module, value)
```

`setInsertBlock` remains the sole current-function state: the insertion block determines the caller
and current return type. A call requires a same-module, non-variadic integer function, exact integer
argument types/count, and arguments available at the insertion point. A valued return requires the
exact current integer result type and an available value. The provider validates the complete
operation before creating an LLVM call or return, and the host clears failed call outputs. All prior
V2 minima remain frozen; an exact Slice 8 provider remains usable for earlier programs, while a
call-shaped program requires the complete new prefix.

The source fixture deliberately needs no `noinline` attribute: the final Slang IR probe retains all
four calls, while final PTX is allowed to inline or algebraically combine them. The pre-libNVVM fake
graph and verified LLVM assembly prove the call/return boundary; differential runtime proves its
semantics. This slice does not add void helpers, external declarations, indirect calls, recursion,
function pointers, pointer/aggregate helper ABI, multiplication or other arithmetic, richer scalar
types, or additional address spaces.

### Slice 10 signed device-pointer element offsets

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source,
    uniform int index)
{
    *(destination + index) = *(source + index);
}
```

The final linked IR keeps the pointer additions as two canonical `IRGetOffsetPtr` instructions.
Each instruction has exactly the base pointer and the signed-i32 `index` parameter as operands. The
destination result has exactly the same read-write device-`int` pointer type as `destination`, and
the source result has exactly the same read-only device-`int` pointer type as `source`; the access
qualifier is not dropped or reconstructed. The source result feeds the established load, the
destination result feeds the established store, and no byte multiplication, cast, or
`IRGetElementPtr` spelling is introduced.

Preflight consumes that linked representation directly before builder discovery. It accepts only
an `IRGetOffsetPtr` with two operands, an exactly equal supported base/result pointer type, and an
available signed-i32 offset that dominates the instruction. The resulting pointer is entered in
the same canonical value map as parameters and scalar results. Existing load/store validation then
remains the sole owner of read-versus-write access checks. In particular, the pointer-offset case
does not walk syntax or operands to rediscover an access qualifier.

The private V2 provider table appends one coherent scalar-pointer-arithmetic prefix:

```text
emitPointerOffset(module, basePointer, elementOffset, outPointer)
```

An exact Slice 9 provider remains compatible and continues to compile every earlier shape. A
pointer-offset module requires the complete new prefix and reaches E52016 after discovery but
before module creation when that prefix is absent. A size inside the appended field or a complete
prefix with a null operation is malformed, while a larger table remains compatible. The host
clears the output before dispatch and again if a provider reports failure after writing a handle,
and the builder identity records `scalar-pointer-arithmetic=0|1` for shader-cache keys.

At the LLVM boundary, the current unterminated insertion block remains the source of caller
ownership. Before mutation the provider requires a live module, a non-null output, a typed,
non-opaque scalar base pointer with a sized pointee, and a scalar integer offset. Both values must
belong to the same module and be available at the insertion point. The provider derives the GEP
element type only from the base pointer and emits ordinary LLVM 14 `CreateGEP`, not
`CreateInBoundsGEP`. A Slang element offset does not establish LLVM's stronger `inbounds`
object/provenance promise, and the negative-index runtime case intentionally starts from an
interior pointer rather than claiming that memory before an allocation is valid.

The provider enforces `pointeeType->isSized()` even though the current public builder constructors
do not expose a safe way to construct an unsized-pointee pointer: `getPointerType` accepts only a
loadable/storable pointee. The guard protects the ABI boundary and future type constructors, but
the current black-box rejection tests do not claim to exercise it with an invented raw LLVM
handle.

This boundary is only signed-i32 element offsetting on the existing device `Ptr<int>` ABI. It does
not add unsigned or wider offsets, other pointee types, `IRGetElementPtr`, arrays, vectors,
aggregates, globals, shared/local/constant/generic address spaces, pointer helper parameters or
results, pointer casts/comparisons/subtraction, byte addressing, bounds checks, or `inbounds`
provenance. Those remain separate producer and ABI decisions.

### Slice 11 fixed device-array element addressing

Consider this example:

```slang
typealias RWIntArray4 = Ptr<int[4], Access::ReadWrite, AddressSpace::Device>;
typealias RIntArray4 = Ptr<int[4], Access::Read, AddressSpace::Device>;

[CUDAKernel]
void computeMain(
    uniform RWIntArray4 destination,
    uniform RIntArray4 source,
    uniform int index)
{
    (*destination)[index] = (*source)[index];
}
```

After the established CUDA passes and `simplifyIR`, the final entry signature is exactly
`Func(Void, Ptr(Array(Int,4),RW,UserPointer), Ptr(Array(Int,4),Read,UserPointer), Int)`. The body
contains two canonical, two-operand `IRGetElementPtr` instructions. Each consumes its array-pointer
parameter and the same signed-i32 `index` parameter. The source result feeds the existing scalar
load, and that load feeds the existing scalar store through the destination result. The emitter
does not turn this representation into `IRGetOffsetPtr`, byte arithmetic, or reconstructed source
syntax.

The pointer-type relation is intentionally semantic rather than whole-type equality. Both bases
point to the same `Array(Int,4)` with `DefaultLayout`; the destination result is a read-write device
`Ptr(Int, ScalarLayout)`, while the source result is a read-only device
`Ptr(Int, ScalarLayout)`. Preflight therefore requires the same address space and access on each
base/result pair, and requires the result pointee to equal the array element type, while allowing
the canonical layout spelling to change. CUDA's canonical buffer-element lowering selects the
`Natural` layout rule for pointer pointees regardless of a source pointer's layout annotation.
Consequently, the storage-shape invariant is the exact `IRArrayType`:
signed-i32 elements, a nonzero `IRIntLit` count that fits `uint32_t`, and exactly its element and
count operands with no custom stride.

The private V2 provider table appends one coherent two-operation array-addressing prefix:

```text
getArrayType(module, elementType, nonzeroCount, outType)
emitArrayElementPointer(module, baseArrayPointer, elementIndex, outPointer)
```

The complete V2 table is 264 bytes on the 64-bit build, while the exact 248-byte Slice 10 prefix and
all older minima remain frozen and compatible for their established programs. A byte count between
those two coherent prefixes, or a complete prefix with either operation null, is malformed; larger
tables remain compatible. An array-shaped program presented to an exact Slice 10 provider reaches
E52016 after discovery but before module creation. The host reports
`scalar-array-addressing=0|1` in the builder identity and clears output handles both before dispatch
and after any failed provider call.

The provider keeps the ABI mechanism more general than the Slang-side policy. `getArrayType`
requires a live module, a non-null output, a same-context element type accepted by
`ArrayType::isValidElementType` and `PointerType::isLoadableOrStorableType`, a sized element, and a
nonzero count. `emitArrayElementPointer` requires a live module, a non-null output, a current
unterminated insertion block, a typed non-opaque pointer in a declared NVVM address space whose
pointee is a nonempty sized array, a scalar integer index, and base/index values owned by and
available in the current function. All validation precedes the only mutation. The operation then
emits ordinary non-`inbounds`
`CreateGEP(arrayType, base, {i32 0, index})`; a Slang subscript does not establish LLVM's stronger
object/provenance promise.

This boundary is only nonempty fixed signed-i32 arrays behind read or read-write device entry-point
pointers, indexed by signed `i32`. It does not add unsized, empty, nested, vector, matrix, struct,
tuple, or non-i32 arrays; array SSA values or aggregate load/store/copy; local allocation, globals,
constant or shared storage; other address spaces; pointer-to-array helper ABI; unsigned or wider
indices; bounds checks; `inbounds` provenance; thread builtins; barriers; atomics; resources; or
libdevice. Those shapes have different canonical producers and remain separate slices.

### Slice 12 signed-i32 multiplication

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x * y;
}
```

After the established CUDA passes and `simplifyIR`, the final entry signature is exactly
`Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int, Int)`. Its body contains one canonical,
two-operand `kIROp_Mul`; the two signed-i32 parameters are its operands, and its signed-i32 result
feeds the established device-pointer store. Preflight admits that instruction directly, validates
both operands through the existing signed-i32 availability and dominance rules, and records the
result in the same canonical value set and provider-value map as the earlier scalar operations. It
does not reconstruct source syntax or introduce an alternate multiplication spelling.

The private V2 provider table appends one dedicated operation:

```text
emitIntegerMultiply(module, left, right, outValue)
```

This operation does not extend `SlangNVVMIntegerBinaryOp_2`. Exact Slice 7 providers implement that
function only for its frozen ADD/SUB domain, so adding a new enum value would silently widen an old
call contract and would still require a separately negotiated marker. The dedicated field makes
multiplication support and dispatch one atomic append-only capability.

The complete V2 table is 272 bytes on the 64-bit build. The exact 264-byte Slice 11 prefix and every
older minimum remain frozen and compatible for their established programs. A size from 265 through
271 bytes inside the new function pointer, or a complete prefix with a null operation, is malformed;
future-larger tables remain compatible and are clamped to the host's known capacity. A
multiplication program presented to an exact Slice 11 provider reaches E52016 after discovery but
before module creation. The host reports `scalar-integer-multiply=0|1` in the builder identity and
clears output handles before dispatch and after a failed or success-without-handle provider call.

At the LLVM boundary, the provider requires a live module, a non-null output, and a current
unterminated insertion block belonging to that module. Both operands must be scalar LLVM integers
of exactly the same type, have valid module, context, and function ownership for their value kind,
and be available at the insertion point under the established same-block ordering and cross-block
dominance rules. All validation precedes the sole mutation, `IRBuilder::CreateMul`. LLVM integer
types are signless; the Slang preflight boundary owns the exact signed-i32 policy.

This boundary is only exact two-operand signed-i32 multiplication over already-supported
parameters, constants, phis, and call results. It does not add unsigned, narrow, wide,
floating-point, vector, or matrix multiplication; multiply-high, overflow, saturation, fused
multiply-add, division, remainder, shifts, bitwise operations, or casts; or any new pointer,
aggregate, storage, resource, libdevice, builtin, barrier, atomic, or wave capability.

### Slice 13 signed-i32 bitwise AND

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x & y;
}
```

The front end resolves `&` as `BuiltinOperationKind::BitAnd`, and ordinary IR lowering constructs
exact `kIROp_BitAnd`. After the established CUDA passes and `simplifyIR`, the final entry signature
is `Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int, Int)`. Its body retains one canonical,
two-operand signed-i32 `kIROp_BitAnd`; the kernel parameters are its operands, and its result feeds
the established device-pointer store. Preflight validates both operands through the existing
signed-i32 availability and dominance rules, then records the result in the canonical value set and
provider-value map. It does not accept `kIROp_ConstexprBitAnd` as a fallback or reconstruct source
syntax.

The private V2 provider table appends one dedicated operation:

```text
emitIntegerBitAnd(module, left, right, outValue)
```

As with multiplication, this operation does not extend `SlangNVVMIntegerBinaryOp_2`. Exact Slice 7
providers implement that callable only for its frozen ADD/SUB domain and reject unknown enum values.
A dedicated terminal field therefore negotiates and dispatches bitwise AND atomically without
widening an older callable's input contract.

The complete V2 table is 280 bytes on the 64-bit build. The exact 272-byte Slice 12 prefix and every
older minimum remain frozen and compatible for their established programs. Sizes 273 through 279
inside the new function pointer, or a complete prefix with a null operation, are malformed;
future-larger tables remain compatible and are clamped to the host's known 280-byte capacity. A
bitwise-AND program presented to an exact Slice 12 provider reaches E52016 after discovery but
before module creation or libNVVM use. The host reports `scalar-integer-bit-and=0|1` in the builder
identity and clears output handles before dispatch and after a failed or success-without-handle
provider call.

At the LLVM boundary, the provider requires a live module, a non-null output, and a current
unterminated insertion block belonging to that module. Both operands must be scalar LLVM integers
of exactly the same type, have valid module, context, and function ownership for their value kind,
and be available at the insertion point under the established same-block ordering and cross-block
dominance rules. All validation precedes the sole mutation, `IRBuilder::CreateAnd`. LLVM integer
types are signless; the Slang preflight boundary owns the exact signed-i32 policy.

This boundary is only exact two-operand signed-i32 bitwise AND over already-supported parameters,
constants, phis, calls, multiplication results, and other signed-i32 producers. It does not add
bitwise OR, XOR, or NOT; shifts or logical operations; unsigned, narrow, wide, vector, matrix, or
aggregate bitwise values; or any new ABI, pointer, storage, resource, builtin, atomic, wave, or
libdevice capability.

### Slice 14 signed-i32 bitwise OR

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x | y;
}
```

The front end resolves `|` as `BuiltinOperationKind::BitOr`, and ordinary IR lowering constructs
exact `kIROp_BitOr`. After the established CUDA passes and `simplifyIR`, the final entry signature
is `Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int, Int)`. Its body retains one canonical,
two-operand signed-i32 `kIROp_BitOr`; the kernel parameters are its operands, and its result feeds
the established device-pointer store. Preflight validates both operands through the existing
signed-i32 availability and dominance rules, then records the result in the canonical value set and
provider-value map. It does not accept `kIROp_ConstexprBitOr` or logical `kIROp_Or` as fallbacks or
reconstruct source syntax.

The private V2 provider table appends one dedicated operation:

```text
emitIntegerBitOr(module, left, right, outValue)
```

The operation does not widen either `SlangNVVMIntegerBinaryOp_2`, whose ADD/SUB domain is frozen,
or the dedicated bitwise-AND callable. A dedicated terminal field negotiates and dispatches
bitwise OR atomically while preserving the exact contracts of all earlier providers.

The complete V2 table is 288 bytes on the 64-bit build. The exact 280-byte Slice 13 prefix and every
older minimum remain frozen and compatible for their established programs. Sizes 281 through 287
inside the new function pointer, or a complete prefix with a null operation, are malformed;
future-larger tables remain compatible and are clamped to the host's known 288-byte capacity. A
bitwise-OR program presented to an exact Slice 13 provider reaches E52016 after discovery but
before module creation or libNVVM use. The host reports `scalar-integer-bit-or=0|1` in the builder
identity and clears output handles before dispatch and after a failed or success-without-handle
provider call.

At the LLVM boundary, the provider requires a live module, a non-null output, and a current
unterminated insertion block belonging to that module. Both operands must be scalar LLVM integers
of exactly the same type, have valid module, context, and function ownership for their value kind,
and be available at the insertion point under the established same-block ordering and cross-block
dominance rules. All validation precedes the sole mutation, `IRBuilder::CreateOr`. LLVM integer
types are signless; the Slang preflight boundary owns the exact signed-i32 policy.

This boundary is only exact two-operand signed-i32 bitwise OR over already-supported parameters,
constants, phis, calls, multiplication results, bitwise-AND results, and other signed-i32
producers. It does not add bitwise XOR or NOT; shifts or logical operations; unsigned, narrow,
wide, vector, matrix, or aggregate bitwise values; or any new ABI, pointer, storage, resource,
builtin, atomic, wave, or libdevice capability. Differential PTX evidence requires matching
`[64, 32, 32]` parameter widths, a token-boundary `or.b32` instruction that cannot be mistaken for
the suffix of `xor.b32`, and an entry-scoped global i32 store; it does not require textual PTX
equality.

### Slice 15 signed-i32 bitwise XOR

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x ^ y;
}
```

The front end resolves `^` as `BuiltinOperationKind::BitXor`, and ordinary IR lowering constructs
exact `kIROp_BitXor`. After the established CUDA passes and `simplifyIR`, the final entry signature
is `Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int, Int)`. Its body retains one canonical,
two-operand signed-i32 `kIROp_BitXor`; the kernel parameters are its operands, and its result feeds
the established device-pointer store. Preflight validates both operands through the existing
signed-i32 availability and dominance rules, then records the result in the canonical value set and
provider-value map. It does not accept `kIROp_ConstexprBitXor` or another logical/comparison opcode
as a fallback or reconstruct source syntax.

The private V2 provider table appends one dedicated operation:

```text
emitIntegerBitXor(module, left, right, outValue)
```

The operation does not widen `SlangNVVMIntegerBinaryOp_2` or reuse the dedicated bitwise-AND or
bitwise-OR callables. Their published domains are frozen. A dedicated terminal field negotiates
and dispatches bitwise XOR atomically while preserving the exact contracts of all earlier
providers.

The complete V2 table is 296 bytes on the 64-bit build. The exact 288-byte Slice 14 prefix and every
older minimum remain frozen and compatible for their established programs. Sizes 289 through 295
inside the new function pointer, or a complete prefix with a null operation, are malformed;
future-larger tables remain compatible and are clamped to the host's known 296-byte capacity. A
bitwise-XOR program presented to an exact Slice 14 provider reaches E52016 after discovery but
before module creation or libNVVM use. The host reports `scalar-integer-bit-xor=0|1` in the builder
identity and clears output handles before dispatch and after a failed or success-without-handle
provider call.

At the LLVM boundary, the provider requires a live module, a non-null output, and a current
unterminated insertion block belonging to that module. Both operands must be scalar LLVM integers
of exactly the same type, have valid module, context, and function ownership for their value kind,
and be available at the insertion point under the established same-block ordering and cross-block
dominance rules. All validation precedes the sole mutation, `IRBuilder::CreateXor`. LLVM integer
types are signless; the Slang preflight boundary owns the exact signed-i32 policy.

This boundary is only exact two-operand signed-i32 bitwise XOR over already-supported parameters,
constants, phis, calls, multiplication results, bitwise-AND/OR results, and other signed-i32
producers. It does not add bitwise NOT; shifts, division, remainder, or logical operations;
unsigned, narrow, wide, vector, matrix, or aggregate bitwise values; or any new ABI, pointer,
storage, resource, builtin, atomic, wave, or libdevice capability. Differential PTX evidence
requires matching `[64, 32, 32]` parameter widths, an exact token-boundary `xor.b32` instruction,
and an entry-scoped global i32 store; it does not require textual PTX equality.

### Slice 16 signed-i32 bitwise NOT

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x)
{
    *destination = ~x;
}
```

The front end resolves `~` as `BuiltinOperationKind::BitNot`, and ordinary IR lowering constructs
exact `kIROp_BitNot`. After the established CUDA passes and `simplifyIR`, the final entry signature
is `Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int)`. Its body retains one canonical,
one-operand signed-i32 `kIROp_BitNot`; the scalar kernel parameter is its operand, and its result
feeds the established device-pointer store. Preflight validates the operand through the existing
signed-i32 availability and dominance rules, then records the result in the canonical value set and
provider-value map. It does not accept `kIROp_ConstexprBitNot`, logical `kIROp_Not`, or a synthesized
XOR as a fallback or reconstruct source syntax.

The private V2 provider table appends one dedicated unary operation:

```text
emitIntegerBitNot(module, value, outValue)
```

The operation does not widen any frozen binary-integer callable. A dedicated terminal field
negotiates and dispatches bitwise NOT atomically while preserving the exact contracts of all
earlier providers.

The complete V2 table is 304 bytes on the 64-bit build. The exact 296-byte Slice 15 prefix and every
older minimum remain frozen and compatible for their established programs. Sizes 297 through 303
inside the new function pointer, or a complete prefix with a null operation, are malformed;
future-larger tables remain compatible and are clamped to the host's known 304-byte capacity. A
bitwise-NOT program presented to an exact Slice 15 provider reaches E52016 after discovery but
before module creation or libNVVM use. The host reports `scalar-integer-bit-not=0|1` in the builder
identity and clears output handles before dispatch and after a failed or success-without-handle
provider call.

At the LLVM boundary, the provider requires a live module, a non-null output, and a current
unterminated insertion block belonging to that module. The operand must be a scalar LLVM integer
with valid module, context, and function ownership for its value kind and be available at the
insertion point under the established same-block ordering and cross-block dominance rules. The
provider extracts this per-value contract into one shared unary integer validator; the established
binary validator composes two unary checks with exact LLVM type equality, preserving every prior
binary-operation rule rather than duplicating or weakening validation. All validation precedes the
sole mutation, `IRBuilder::CreateNot`. LLVM represents this as `xor i32` with an all-ones (`-1`)
operand. LLVM integer types are signless; the Slang preflight boundary owns the exact signed-i32
policy.

This boundary is only exact one-operand signed-i32 bitwise NOT over already-supported parameters,
constants, phis, calls, arithmetic and bitwise results, and other signed-i32 producers. It does not
add arithmetic negation; shifts, division, remainder, or logical operations; unsigned, narrow,
wide, vector, matrix, or aggregate bitwise values; or any new ABI, pointer, storage, resource,
builtin, atomic, wave, or libdevice capability. Differential PTX evidence requires matching
`[64, 32]` parameter widths, an exact token-boundary `not.b32` instruction, and an entry-scoped
global u32 store; it does not require textual PTX equality.

### Slice 17 signed-i32 arithmetic negation

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x)
{
    *destination = -x;
}
```

The front end resolves unary `-` as `BuiltinOperationKind::Neg`, and ordinary IR lowering
constructs exact `kIROp_Neg`. The measured post-Slice-16 baseline retains the final entry signature
`Func(Void, Ptr(Int,RW,UserPointer,DefaultLayout), Int)`, one canonical one-operand signed-i32
`neg(%x)`, and its device-pointer store consumer through the final `simplifyIR`. Preflight validates
the operand through the established signed-i32 availability and dominance rules, then records the
result in the canonical value set and provider-value map. It does not accept `kIROp_ConstexprNeg`,
synthesize Slang-IR subtraction from zero, or reconstruct source syntax as a fallback.

The private V2 provider table appends one dedicated unary operation:

```text
emitIntegerNegate(module, value, outValue)
```

The operation does not widen any frozen binary-integer callable or the dedicated BitNot callable.
A dedicated terminal field negotiates and dispatches arithmetic negation atomically while
preserving the exact contracts of all earlier providers.

The complete V2 table is 312 bytes on the 64-bit build. The exact 304-byte Slice 16 prefix and every
older minimum remain frozen and compatible for their established programs. Sizes 305 through 311
inside the new function pointer, or a complete prefix with a null operation, are malformed;
future-larger tables remain compatible and are clamped to the host's known 312-byte capacity. A
Neg program presented to an exact Slice 16 provider must reach E52016 after discovery but before
module creation or libNVVM use. The host reports `scalar-integer-negate=0|1` in the builder identity
and clears output handles before dispatch and after a failed or success-without-handle provider
call.

At the LLVM boundary, the provider requires a live module, a non-null output, and a current
unterminated insertion block belonging to that module. The operand must satisfy Slice 16's shared
unary integer validator: scalar LLVM integer type, valid module/context/function ownership, and
availability under the established same-block ordering and cross-block dominance rules. All
validation precedes the sole mutation, plain `IRBuilder::CreateNeg` without `nsw` or `nuw`. LLVM
represents this operation as `sub i32 0, %x`; the absence of no-wrap flags preserves Slang's
documented wrapping integer arithmetic, including `-INT_MIN == INT_MIN`.

This boundary is only exact one-operand signed-i32 arithmetic negation over already-supported
parameters, constants, phis, calls, arithmetic and bitwise results, and other signed-i32 producers.
It does not add unsigned, narrow, wide, floating-point, vector, matrix, or aggregate negation;
shifts, division, remainder, or logical operations; or any new ABI, pointer, storage, resource,
builtin, atomic, wave, or libdevice capability. The measured explicit NVRTC baseline exposes
`[64, 32]`, exact `neg.s32`, `cvta.to.global.u64`, and `st.global.u32`; the pre-change direct route
stops deterministically at E52017 `'neg'`. After implementation, integrated direct NVVM preserves
the same widths, exact `neg.s32`, and global u32 store while using the raw pointer. Neither route
uses `sub.s32` or `not.b32` as an alternate spelling.

### Slice 18 toolkit-matched libdevice and floating-point policy

Slice 18 is a downstream compiler and toolkit-policy boundary. A caller requests libdevice through
the terminal versioned compile-options field:

```text
uintptr_t requiresCUDADeviceLibrary = 0
```

The naturally aligned pointer-sized storage prevents the field from reusing an older structure's
tail padding. Zero means false and any nonzero value means true. A caller whose compatible prefix
predates the field receives zero, so every established integer module continues to compile without
requiring, reading, or adding libdevice. Compiler-version identity is independent of compile
demand and may stat the exact coherent libdevice path when a selected toolkit root is known. The
flag is an explicit semantic demand; the compiler does not scan LLVM text or bitcode for symbol
spellings.

When the field is nonzero, all source and floating-policy validation plus the exact
`<selected-root>/nvvm/libdevice/libdevice.10.bc` read completes before program creation. The file is
opaque binary data, including embedded NULs. After creation, the compiler adds the user module
normally as `slang-nvvm-input`, then adds the exact library bytes as `libdevice.10.bc` through
`nvvmLazyAddModuleToProgram`. Ordinary `nvvmAddModuleToProgram` is used for the library only when
the optional lazy-add symbol is absent; a failed lazy add is surfaced and never retried eagerly.
Verification and compilation follow with the one canonical option vector. Every failure after
creation returns the usual diagnostic artifact and destroys the program exactly once.

The demonstrable floating-point surface in this slice is a compiler-level NVVM module that
declares and calls an exact libdevice function such as `__nv_sinf`. This does not extend the direct
linked-Slang-IR emitter. Raw `float` and `Ptr<float>` entry parameters, floating-point constants,
loads, stores, arithmetic, phis, helpers, and intrinsics remain unsupported there. The direct
`sin(float)` negative case stops while collecting the call closure because the target-intrinsic
helper has an unsupported float result, before provider discovery. It therefore proves only that
the f32/helper boundary stays closed; it does not exercise or claim a `GenericAsm` matcher. No such
matcher, V2 builder operation, or provider-table byte is added by Slice 18. A later semantic
intrinsic-producer boundary must request libdevice when it emits an actual declaration and call.

The deterministic request, path, module-order, fallback, diagnostic, identity, and option contracts
above are the implemented Slice 18 architecture. The focused NVVM suite passed 132/132. The real
toolkit test produced a named-entry kernel with a global store and no unresolved `.extern .func`,
the same CUDA 12.9 root's `ptxas` accepted it for `sm_75`, and the RTX 5090 runtime results matched
host `sinf` within `2e-6`. Preservation passed 1/1 parser, 2/2 routing/hash, 1/1 unsupported
boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

### Slice 19 relaxed global signed-i32 atomic add

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> previous,
    uniform int delta)
{
    int oldValue;
    InterlockedAdd(*destination, delta, oldValue);
    *previous = oldValue;
}
```

The public `InterlockedAdd` wrapper force-inlines through `__atomic_add`, whose intrinsic opcode is
canonical `kIROp_AtomicAdd`. The measured final linked Slang IR contains the exact operation
`atomicAdd(destination, delta, 0)`: `destination` is the established read-write device `Ptr<int>`,
`delta` and the result are signed `i32`, and the literal final operand `0` is
`MemoryOrder::Relaxed`. The operation returns the value stored immediately before the update, and
the example's ordinary store consumes that result through `previous`. This measured instruction is
the semantic source of truth. Direct emission must not recognize an intrinsic name, reconstruct
source syntax, or reinterpret a load/add/store sequence as atomic.

The CUDA reference is also exact. CUDA's unsuffixed `atomicAdd` is relaxed at device scope, and the
measured CUDA 12.9 NVRTC output uses `atom.global.add.u32`. The omitted PTX semantic and scope
qualifiers preserve that relaxed/device contract. The `.u32` spelling does not widen the source
boundary to unsigned Slang values; LLVM integers and this PTX addition are signless at the bit
operation boundary while Slang preflight owns the signed-i32 restriction.

The private V2 provider table appends one deliberately exact operation:

```text
emitRelaxedGlobalI32AtomicAdd(module, pointer, value, outOriginalValue)
```

This callable accepts only a same-module typed address-space-1 pointer to `i32` and a same-module
`i32` value available at the current unterminated insertion point. It must clear the output before
dispatch and failure, validate module/context/function ownership and availability before mutation,
and emit one LLVM `atomicrmw add` with `Align(4)`, `AtomicOrdering::Monotonic`, and LLVM's default
System sync-scope spelling. LLVM `monotonic` is the representation of the measured Relaxed policy;
the default scope adds no target-specific sync-scope spelling, and libNVVM maps this form to the
unsuffixed device-scope PTX atomic. `atomicrmw` returns the original value for the direct value map.
The ABI intentionally carries no configurable order, alignment, address space, type, or sync
scope.

LLVM 14 bitcode is not generally backward-readable by libNVVM's LLVM 7 reader. The first
`atomicrmw` exposed that the LLVM 14 writer uses current atomic record 59 while the older reader
expects legacy record 38. Earlier scalar operations happened to use compatible records and did not
prove whole-dialect compatibility. CUDA 12.9 rejects the LLVM 14 atomic bitcode with producer
LLVM 14.0.6 / reader LLVM 7.0.1 before verification.

Slice 19 therefore makes the wire dialect explicit. It appends
`serializeNVVMIR20AssemblyWithDiagnostics` immediately after the atomic operation and treats both
function pointers as one coherent capability. The complete V2 prefix grows from 312 to 328 bytes
on x64 and from 176 to 184 bytes on x86. Every size inside that two-pointer suffix is malformed,
and a full table with either null pointer is rejected. Future-larger tables remain compatible and
are clamped. The host includes `nvvm-ir-2.0-assembly=0/1` in provider identity.

This negotiation preserves old providers. A Slice 17 or earlier V2 provider continues to receive
verified LLVM bitcode for the programs it supports. Only a complete Slice 19 provider receives the
new `SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY` format. Generic assembly serialization
still returns raw LLVM 14 text, including explicit atomic alignment; the direct path neither
content-sniffs nor retries after libNVVM failure.

The provider-owned compatibility writer has only three conversions demonstrated by direct libNVVM
probes:

1. Each function parameter receives a stable `slangParameterN` name when the LLVM function is
   declared. LLVM 14 otherwise prints explicit numeric parameter declarations that the LLVM 7
   parser rejects, while named parameters are valid in both dialects.
2. For the negotiated writer only, the provider removes LLVM 14's terminal `, align 4` spelling
   from `atomicrmw`. LLVM 7 gives this operation natural alignment and rejects that newer suffix.
3. LLVM 14 prints canonical float negation as `fneg`, which CUDA 12.9 libNVVM's NVVM-2.0 parser
   rejects. For the negotiated writer only, one validated unflagged scalar `fneg float` becomes the
   legacy spelling `fsub float -0.0, value` used by the older dialect.

The latter two conversions are not unbounded text replacements. Before printing, the provider
walks the LLVM instructions and requires every semantic atomic to be non-volatile AS1 i32 ADD with
alignment four, Monotonic ordering, and System sync scope, and every rewritten negation to have one
same-typed scalar `float` operand and result. It rewrites only exact instruction-result lines, and
each rewritten line count must equal its semantic instruction count. Any future shape or printer
spelling fails with `SLANG_E_NOT_AVAILABLE`. The LLVM module, generic assembly, and bitcode remain
unchanged.

Numba provides independent production evidence for this architecture: it submits textual LLVM IR
to libNVVM and performs version-specific normalization at that boundary. NVIDIA deprecates textual
input, so this remains an experimental compatibility bridge rather than the production-readiness
answer. The builder ABI keeps lowering independent of the wire encoding, allowing a later
LLVM-7-compatible bitcode writer to replace the text serializer without changing Slang IR
traversal.

A warmed synthetic audit found no material text overhead at this boundary. Modules with 1, 100,
and 500 empty kernels took approximately 3.1, 39, and 200 ms end-to-end through text and 3.1, 39,
and 268 ms through LLVM 14 bitcode on this machine; serialization itself stayed below 1.5 ms for
text and 2.8 ms for bitcode. Compilation dominates both paths, and the 500-kernel difference is
reader-path variability rather than evidence that text is intrinsically faster.

An atomic-add program presented to the exact 312-byte Slice 17 provider reaches E52016 after
discovery but before builder-module creation. An integer-negate program on that same provider still
compiles and the captured libNVVM input begins with LLVM bitcode magic, proving that capability
negotiation—not atomic-content coupling—selects the wire format.

This boundary is only canonical Relaxed signed-i32 atomic add through an already-supported
read-write device-i32 pointer, including preservation of the returned original value. The primary
proof uses the raw `Ptr<int>` entry-point ABI, but preflight does not invent an origin restriction
for a canonical offset or array-element pointer that the direct backend already accepts. Atomic
load/store, subtract, exchange, compare-exchange, min/max, bitwise atomics,
increment/decrement, and reductions remain unsupported. So do Acquire, Release, AcquireRelease,
and SeqCst orders; unsigned, narrow, wide, floating-point, vector, matrix, aggregate, and resource
atomics; generic, shared, constant, and local storage; fences, barriers, thread builtins, and
wave/subgroup operations. This slice adds no new pointer-construction capability. Waves remain an
independent Bucket 8 boundary because lane semantics, convergence, intrinsic selection, and
executable evidence do not share this atomic producer contract.

The resulting LLVM module verifies before serialization. CUDA 12.9 libNVVM accepts the normalized
assembly and produces token-safe `atom.global.add.u32`; matching-root `ptxas` accepts both direct
and NVRTC PTX. On an RTX 5090, 2,048 threads adding one to one initialized device integer produce
the exact launch count through both routes, and the returned-old-value fixture preserves the
ordinary store dependency. The negative matrix rejects adjacent operations, types, orders, address
spaces, ownership errors, and unavailable values before mutation or provider discovery as
appropriate. The final Release NVVM prefix passed 140/140. Preservation passed 1/1 parser, 2/2
routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime
dispatch. The provider still exports only its V1/V2 getters and has no process-visible LLVM DLL
dependency.

### Slice 20 isolated LLVM 7.0.1 bitcode experiment

Slice 20 tested whether the LLVM-free provider graph could use the bitcode dialect named by NVVM
IR 2.0 rather than the LLVM 14 text bridge. The isolated `experiment/nvvm-llvm7-bitcode` branch
builds exact upstream LLVM 7.0.1 as a statically linked provider and uses generic verified bitcode
for all thirteen module shapes implemented through Slice 19, including the formerly incompatible
`atomicrmw`. CUDA 12.9 libNVVM, matching-root `ptxas`, differential PTX, and the RTX 5090 atomic
old-value runtime fixture all pass. The writer therefore provides an executable compatibility
oracle and proves that bitcode compatibility is technically achievable without changing Slang IR
lowering or the LLVM-free provider ABI.

That result does not change this branch's production baseline. LLVM 7 cannot configure with CMake
4, needs an older CMake frontend and an ancient dependency/update/security policy, and brings a
larger transitive static-library maintenance surface. The experiment stays isolated while feature
work continues with exact LLVM 14.0.6 plus the audited NVVM IR 2.0 text serializer. Choosing the
LLVM 7 writer, keeping the negotiated text bridge, or building a smaller purpose-built writer is a
separate production dependency decision.

### Slice 21 signed-i32 equality

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left == right ? 1 : 0;
}
```

The measured final linked Slang IR retains one exact `kIROp_Eql` with the two signed-`i32`
parameters as operands and the canonical Boolean type as its result. The existing conditional
lowering consumes that result, each arm contributes an exact integer constant, the merge block
selects them through the established integer phi, and the ordinary device-pointer store consumes
the phi. This canonical opcode and value graph are the source of truth. Direct emission does not
recognize source syntax, infer equality from branch topology, or synthesize it from subtraction.

The private V2 provider table appends one dedicated operation after the complete Slice 19 prefix:

```text
emitIntegerEqual(module, left, right, outValue)
```

The complete table is 336 bytes on x64 and 188 bytes on x86. The exact 328-byte/184-byte Slice 19
prefix remains valid for every established program but reports `scalar-integer-equal=0`; an
equality program reaches E52016 after discovery and before module creation. Every byte count inside
the new function pointer and a complete table with a null callback are malformed. Future-larger
tables are accepted and clamped, and the host clears stale output handles before dispatch and after
failure or success without a result.

At the provider boundary, equality shares the established binary integer validation path with
signed less-than. Both operands must be available same-module, same-function scalar LLVM integers
of one exact type at the current unterminated insertion point. Complete ownership, ordering,
dominance, and type validation precedes the sole mutation. The selected predicate is exact LLVM
`ICMP_EQ`, yielding an `i1`; Slang preflight, not LLVM's signless integer type, owns the signed-i32
restriction. The resulting Boolean handle is recorded in the existing value map and can feed the
already-supported Boolean branch consumer without adding Boolean parameter, storage, return, or
phi ABI.

This boundary does not add inequality, ordered comparisons, unsigned/wide/floating-point/pointer
equality, vectors, matrices, aggregates, resources, or new pointer and storage shapes. Unsigned,
wide, and floating equality stop at their unsupported entry parameters; pointer equality reaches
the exact signed-i32 operand check. All four stop before provider discovery. Provider-level invalid
operations also reject missing or terminated insertion points, null outputs, wrong or mismatched
types, cross-module/cross-function handles, and unavailable or non-dominating values before
mutation.

CUDA 12.9 direct NVVM and NVRTC PTX expose matching `[64, 32, 32]` parameter widths, token-safe
32-bit equality comparison, and the global 32-bit store. The selected toolkit's `ptxas` accepts
both outputs. On the RTX 5090, both routes return one for `0 == 0` and `-7 == -7`, and zero for
`-7 == 7` and `INT_MIN == INT_MAX`. The Release focused NVVM suite passes 148/148; preservation
passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA
compile/pass-through, and 1/1 runtime dispatch. The provider still exports only the V1/V2 getters
and has no process-visible LLVM DLL dependency.

### Slice 22 signed-i32 inequality

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left != right ? 1 : 0;
}
```

The measured final linked Slang IR retains one exact `kIROp_Neq` with the two signed-`i32`
parameters as operands and the canonical Boolean type as its result. The established conditional,
integer-constant, phi, and device-store graph consumes it directly. This canonical opcode and
value graph are the source of truth. Direct emission does not recognize source syntax, invert an
equality result, infer inequality from branch topology, or synthesize it from subtraction.

The private V2 provider table appends one dedicated operation after the complete Slice 21 prefix:

```text
emitIntegerNotEqual(module, left, right, outValue)
```

The complete table is 344 bytes on x64 and 192 bytes on x86. The exact 336-byte/188-byte Slice 21
prefix remains valid for every established program and reports `scalar-integer-not-equal=0`; an
inequality program reaches E52016 after discovery and before module creation. Every byte count
inside the appended function pointer and a complete table with a null callback are malformed.
Future-larger tables are accepted and clamped, and the host clears stale output handles before
dispatch and after failure or success without a result.

At the provider boundary, inequality uses the same established binary integer validator as
signed less-than and equality. Both operands must be available same-module, same-function scalar
LLVM integers of one exact type at the current unterminated insertion point. Complete ownership,
ordering, dominance, and type validation precedes the sole mutation. The selected predicate is
exact LLVM `ICMP_NE`, yielding an `i1`; Slang preflight owns the signed-i32 restriction and
canonical Boolean result. The result feeds the existing Boolean branch consumer without adding a
Boolean parameter, storage, return, or phi ABI.

This boundary does not add ordered comparisons, unsigned/wide/floating-point/pointer inequality,
vectors, matrices, aggregates, resources, or new pointer and storage shapes. Unsigned, wide, and
floating inequality stop at their unsupported entry parameters; pointer inequality reaches the
exact signed-i32 operand check. All four stop before provider discovery. Provider-level invalid
operations reject missing or terminated insertion points, null outputs, wrong or mismatched
types, cross-module/cross-function handles, and unavailable or non-dominating values before
mutation.

CUDA 12.9 direct NVVM and NVRTC PTX expose matching `[64, 32, 32]` parameter widths, token-safe
32-bit equality-predicate comparisons, and the global 32-bit store. The selected toolkit's
`ptxas` accepts both outputs. On the RTX 5090, both routes return zero for `0 != 0` and
`-7 != -7`, and one for `-7 != 7` and `INT_MIN != INT_MAX`. The Release focused NVVM suite passes
156/156; preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3 sampler,
2/2 CUDA compile/pass-through, and 1/1 runtime dispatch. The provider still exports only the V1/V2
getters and has no process-visible LLVM DLL dependency.

### Slice 23 signed-i32 greater-than

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left > right ? 1 : 0;
}
```

The measured final linked Slang IR retains one exact `kIROp_Greater` with the two signed-`i32`
parameters as operands and the canonical Boolean type as its result. The established conditional,
integer-constant, phi, and device-store graph consumes it directly. This canonical producer is an
intentional input shape, not an alternative spelling that an earlier phase should repair: final
linking does not rewrite it to a signed less-than with reversed operands. Removing the exact
consumer case restores E52017 at `cmpGT`, so direct emission owns this opcode without recognizing
source syntax, reversing operands, or adding a fallback equivalence.

The private V2 provider table appends one dedicated operation after the complete Slice 22 prefix:

```text
emitIntegerSignedGreaterThan(module, left, right, outValue)
```

The complete table is 352 bytes on x64 and 196 bytes on x86. The exact 344-byte/192-byte Slice 22
prefix remains valid for every established program and reports
`scalar-integer-signed-greater-than=0`; a greater-than program reaches E52016 after discovery and
before module creation. Every byte count inside the appended function pointer and a complete table
with a null callback are malformed. Future-larger tables are accepted and clamped, and the host
clears stale output handles before dispatch and after failure or success without a result.

At the provider boundary, greater-than uses the same established binary integer validator as
signed less-than, equality, and inequality. Both operands must be available same-module,
same-function scalar LLVM integers of one exact type at the current unterminated insertion point.
Complete ownership, ordering, dominance, and type validation precedes the sole mutation. The
selected predicate is exact LLVM `ICMP_SGT`, yielding an `i1`; Slang preflight owns the signed-i32
restriction and canonical Boolean result. The result feeds the existing Boolean branch consumer
without adding a Boolean parameter, storage, return, or phi ABI.

This boundary does not add less-equal, greater-equal, unsigned/wide/floating-point/pointer ordered
comparison, vectors, matrices, aggregates, resources, or new pointer and storage shapes. Unsigned,
wide, and floating greater-than stop at their unsupported entry parameters; pointer greater-than
reaches the exact signed-i32 operand check. All four stop before provider discovery. Provider-level
invalid operations reject missing or terminated insertion points, null outputs, wrong or
mismatched types, cross-module/cross-function handles, and unavailable or non-dominating values
before mutation.

CUDA 12.9 direct NVVM and NVRTC PTX expose matching `[64, 32, 32]` parameter widths, signed ordered
32-bit comparison predicates, and the global 32-bit store. The selected toolkit's `ptxas` accepts
both outputs. On the RTX 5090, both routes return zero for `0 > 0`, `-7 > -7`, `-7 > 7`, and
`INT_MIN > INT_MAX`, and one for `7 > -7` and `INT_MAX > INT_MIN`. The Release focused NVVM suite
passes 164/164; preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3
sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch. The provider still exports only
the V1/V2 getters and has no process-visible LLVM DLL dependency.

### Slice 24 signed-i32 less-than-or-equal

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left <= right ? 1 : 0;
}
```

The measured final linked Slang IR retains one exact `kIROp_Leq` with the two signed-`i32`
parameters as operands and the canonical Boolean type as its result. The established conditional,
integer-constant, phi, and device-store graph consumes it directly. This canonical producer is an
intentional input shape, not an alternative spelling that an earlier phase should repair: final
linking does not reconstruct it from strict less-than and equality, negate greater-than, or reverse
its operands. Removing the exact consumer case restores E52017 at `cmpLE`, so direct emission owns
this opcode without recognizing source syntax or adding a fallback equivalence.

The private V2 provider table appends one dedicated operation after the complete Slice 23 prefix:

```text
emitIntegerSignedLessEqual(module, left, right, outValue)
```

The complete table is 360 bytes on x64 and 200 bytes on x86. The exact 352-byte/196-byte Slice 23
prefix remains valid for every established program and reports
`scalar-integer-signed-less-equal=0`; a less-than-or-equal program reaches E52016 after discovery
and before module creation. Every byte count inside the appended function pointer and a complete
table with a null callback are malformed. Future-larger tables are accepted and clamped, and the
host clears stale output handles before dispatch and after failure or success without a result.

At the provider boundary, less-than-or-equal uses the established binary integer validator. Both
operands must be available same-module, same-function scalar LLVM integers of one exact type at the
current unterminated insertion point. Complete ownership, ordering, dominance, and type validation
precedes the sole mutation. The selected predicate is exact LLVM `ICMP_SLE`, yielding an `i1`;
Slang preflight owns the signed-i32 restriction and canonical Boolean result. The result feeds the
existing Boolean branch consumer without adding a Boolean parameter, storage, return, or phi ABI.

This boundary does not add greater-equal, unsigned/wide/floating-point/pointer ordered comparison,
vectors, matrices, aggregates, resources, or new pointer and storage shapes. Unsigned, wide, and
floating less-equal stop at their unsupported entry parameters; pointer less-equal reaches the
exact signed-i32 operand check. All four stop before provider discovery. Provider-level invalid
operations reject missing or terminated insertion points, null outputs, wrong or mismatched types,
cross-module/cross-function handles, and unavailable or non-dominating values before mutation.

CUDA 12.9 direct NVVM and NVRTC PTX expose matching `[64, 32, 32]` parameter widths, signed ordered
32-bit comparison predicates, and the global 32-bit store. The selected toolkit's `ptxas` accepts
both outputs. On the RTX 5090, both routes return one for `0 <= 0`, `-7 <= -7`, `-7 <= 7`, and
`INT_MIN <= INT_MAX`, and zero for `7 <= -7` and `INT_MAX <= INT_MIN`. The Release focused NVVM
suite passes 172/172; preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary,
3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch. The provider still exports
only the V1/V2 getters and has no process-visible LLVM DLL dependency.

### Slice 25 signed-i32 greater-than-or-equal

Consider this example:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left >= right ? 1 : 0;
}
```

The measured final linked Slang IR retains one exact `kIROp_Geq` with the two signed-`i32`
parameters as operands and the canonical Boolean type as its result. The established conditional,
integer-constant, phi, and device-store graph consumes it directly. This canonical producer is an
intentional input shape, not an alternative spelling that an earlier phase should repair: final
linking does not reconstruct it from strict greater-than and equality, negate less-than, or reverse
its operands. Removing the exact consumer case restores E52017 at `cmpGE`, so direct emission owns
this opcode without recognizing source syntax or adding a fallback equivalence.

The private V2 provider table appends one dedicated operation after the complete Slice 24 prefix:

```text
emitIntegerSignedGreaterEqual(module, left, right, outValue)
```

The complete table is 368 bytes on x64 and 204 bytes on x86. The exact 360-byte/200-byte Slice 24
prefix remains valid for every established program and reports
`scalar-integer-signed-greater-equal=0`; a greater-than-or-equal program reaches E52016 after
discovery and before module creation. Every byte count inside the appended function pointer and a
complete table with a null callback are malformed. Future-larger tables are accepted and clamped,
and the host clears stale output handles before dispatch and after failure or success without a
result.

At the provider boundary, greater-than-or-equal uses the established binary integer validator.
Both operands must be available same-module, same-function scalar LLVM integers of one exact type
at the current unterminated insertion point. Complete ownership, ordering, dominance, and type
validation precedes the sole mutation. The selected predicate is exact LLVM `ICMP_SGE`, yielding
an `i1`; Slang preflight owns the signed-i32 restriction and canonical Boolean result. The result
feeds the existing Boolean branch consumer without adding a Boolean parameter, storage, return, or
phi ABI.

This boundary does not add unsigned/wide/floating-point/pointer ordered comparison, vectors,
matrices, aggregates, resources, or new pointer and storage shapes. Unsigned, wide, and floating
greater-equal stop at their unsupported entry parameters; pointer greater-equal reaches the exact
signed-i32 operand check. All four stop before provider discovery. Provider-level invalid
operations reject missing or terminated insertion points, null outputs, wrong or mismatched types,
cross-module/cross-function handles, and unavailable or non-dominating values before mutation.

CUDA 12.9 direct NVVM and NVRTC PTX expose matching `[64, 32, 32]` parameter widths, signed ordered
32-bit comparison predicates, and the global 32-bit store. The selected toolkit's `ptxas` accepts
both outputs. On the RTX 5090, both routes return one for `0 >= 0`, `-7 >= -7`, `7 >= -7`, and
`INT_MAX >= INT_MIN`, and zero for `-7 >= 7` and `INT_MIN >= INT_MAX`. The Release focused NVVM
suite passes 180/180; preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary,
3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch. The provider still exports
only the V1/V2 getters and has no process-visible LLVM DLL dependency.

### Slice 26 raw `RWStructuredBuffer<int>` storage

Consider this raw CUDA kernel:

```slang
[CUDAKernel]
void computeMain(RWStructuredBuffer<int> destination, uniform int index)
{
    destination[index] = 42;
}
```

Final linking retains one exact `HLSLRWStructuredBufferType(Int, DefaultBufferLayout)` entry-point
parameter and one `kIROp_RWStructuredBufferGetElementPtr(destination, index)`. The result is the
canonical generic, read-write, scalar-buffer-layout pointer to signed `i32`, and the established
constant/store graph consumes it directly. This is an intentional raw CUDA launch-value shape, not
malformed IR that an earlier pass should flatten or legalize. Removing the exact parameter and
producer cases restores E52017 at the entry parameter and element-pointer operation.

A conventional shader global has a different canonical shape. For example,
`RWStructuredBuffer<int> destination;` under a `[numthreads]` entry point retains a
`ConstantBuffer<GlobalParams>`, `get_field_addr`, load, and then the same resource element-pointer
operation. NVRTC places that parameter block in a `SLANG_globalParams` constant symbol instead of
the raw kernel parameter list. Slice 26 therefore accepts only the measured raw `[CUDAKernel]`
shape; it does not treat the conventional representation as an equivalent spelling.

The private V2 provider table appends one coherent two-operation capability after Slice 25:

```text
getRawRWStructuredBufferI32Type(module, outType)
emitRawRWStructuredBufferI32ElementPointer(module, buffer, index, outPointer)
```

The complete table is 384 bytes on x64 and 212 bytes on x86. Exact 368/204-byte Slice 25 providers
remain usable for all established programs and report `raw-rw-structured-buffer-i32=0`; a resource
program reaches E52016 after provider discovery and before module creation. Sizes 369 through 383
on x64, sizes 205 through 211 on x86, and complete tables missing either callback are malformed.
Future-larger providers are accepted and clamped. Both wrappers clear stale handles before
dispatch and after failure or success without a result.

The provider owns one structural source of truth for the raw CUDA ABI: the naturally aligned LLVM
aggregate `{ i32 addrspace(1)*, i64 }`, matching the CUDA prelude's data pointer followed by
`size_t count`. The element-pointer operation requires that exact aggregate, an exact i32 index,
same-module ownership, same-function availability, dominance, and a current unterminated insertion
point before mutation. It then emits one `extractvalue` for field zero and one ordinary,
non-`inbounds` `getelementptr i32`. The existing four-byte signed-i32 store consumes that AS1
pointer; no text manipulation, source-syntax reconstruction, aggregate flattening, or fallback is
involved.

Conventional globals, read-only `StructuredBuffer<int>`, unsigned and floating-point resource
elements, reads or atomics through even an otherwise supported raw read-write resource pointer,
and neighboring address operations stop at exact preflight checks before provider discovery.
Provider-level null, wrong-type, cross-module, cross-function, unavailable, and terminated-block
calls clear their outputs and insert no resource address instructions.

Direct NVVM and NVRTC both expose `.param .align 8 .b8[16]` followed by the signed-i32 index
parameter, load the first u64 data-pointer field, scale the signed i32 index, and issue one global
u32 store. The direct PTX is 611 bytes and the NVRTC reference is 8,559 bytes; size is recorded as
evidence, not a performance claim. CUDA 12.9 `ptxas` accepts both with four registers, no barriers,
no stack or spills, and 372 bytes of `cmem[0]`; NVRTC additionally reports its module-level gmem and
`cmem[4]`. On the RTX 5090, both routes launch the exact 16-byte `{device pointer, count}` argument
and store 42 into the one-element allocation. The Release focused NVVM suite passes 188/188 and
the preservation matrix passes 10/10. The provider still exports only the V1/V2 getters and has no
process-visible LLVM DLL dependency.

### Slice 27 test ownership

Slice 27 changes no backend behavior. It separates the former 25,941-line NVVM unit-test monolith
into four registered-test owners: provider-builder ABI and invalid-operation coverage, fake direct
Slang emitter topology and capability gating, real differential PTX/`ptxas`/runtime integration,
and downstream compiler/loader/libdevice policy. A shared internal support header retains the
existing anonymous-namespace fixtures so each translation unit owns private fake state; it
registers no tests and exports no test ABI.

The exact sorted set of 188 registered test names is unchanged, including every name referenced by
the capability ledger. Its pre/post SHA-256 is
`c197159202001f39765394b2399146398d0c4534803864b3ea44cc694827ac78`. The focused Release suite
passes 188/188 and the Debug preservation matrix passes 10/10. This slice deliberately preserves
the historical per-operation fake representation. Slice 28 can now change provider tests without
mixing compiler/runtime ownership, and Slice 30 owns replacing that fake representation and the
duplicated scalar runners after the generic provider surface is established.

### Slice 28 generic V3 provider boundary

Slice 28 freezes the complete 384-byte x64/212-byte x86 V2 table. V3 composes that immutable table
as its compatibility core instead of copying its lifecycle, CFG, memory, atomic, and resource
callbacks into a second spelling. Forward growth moves to three typed scalar families:

```text
emitIntegerUnary(operation, value, outValue)
emitIntegerBinary(operation, left, right, outValue)
emitIntegerCompare(operation, left, right, outValue)
```

The Slang-owned operation values cover the established unary NOT/negate, binary
add/subtract/multiply/AND/OR/XOR, and signed-less/equal/not-equal/signed-greater/signed-less-equal/
signed-greater-equal operations. LLVM enum values and objects remain provider-private. Each V3
dispatcher calls the same validated LLVM producer retained by V2, and unknown operations clear the
output and fail before mutation. Adding another same-shaped integer operation therefore adds an
enum value, semantic feature mapping, provider switch case, and tests; it does not add another ABI
field, host wrapper method, V2 minimum-prefix constant, or identity field.

Four fixed 64-bit words carry semantic availability independently from table layout. Bits 0 through
19 represent the established scalar memory, control flow, SSA, function, pointer/array addressing,
six post-control-flow integer operation families, relaxed global-i32 atomic add, NVVM-2.0 assembly,
five post-control-flow comparisons, and raw `RWStructuredBuffer<int>` storage. The current provider
advertises word zero as `1048575` and the remaining words as zero. A V2 compatibility adapter
synthesizes the same bits from each exact coherent prefix, so a hole in one V3 feature does not hide
later independent features while every historical V2 subset retains its old behavior.

The complete V3 table is 448 bytes on x64. Its x86 layout is 272 bytes because four bytes align the
feature words after the 212-byte V2 core and four bytes pad the final 268-byte callback minimum.
The host probes V3 first and falls back only when the V3 symbol is absent. A present V3 table with a
bad version, incomplete frozen core, or missing generic callback fails without trying V2. Future-
larger V3 tables are clamped to the locally understood 448/272-byte layout, unknown advertised bits
are retained, and a requirement containing a bit the host does not understand is rejected.

The direct preflight now accumulates a feature set rather than the numerically greatest historical
slice, and scalar emission uses the three generic facade methods. The Release focused suite passes
192/192, including the complete real direct/NVRTC PTX comparison, `ptxas`, and RTX 5090 runtime
matrix. Debug preservation passes 10/10. The provider exports exactly the V1, V2, and V3 getters;
PE inspection reports only operating-system dependencies and no process-visible LLVM DLL.

### Slice 29 centralized type legalization

Slice 29 gives canonical linked Slang IR types one construction owner,
`NVVMTypeLoweringContext`. Each direct emission creates exactly one context after creating its
provider module, so every cached handle is module-owned and is discarded before that module is
destroyed. Function results and parameters state whether they belong to the entry point or a helper;
ordinary values state the value-use contract. The context rejects any type that preflight should not
have admitted instead of repairing an alternative spelling in the emitter.

The source cache is keyed by exact canonical `IRType*`. It recursively maps the established graph:
entry-point `void`, helper/value signed `int` to signless LLVM `i32`, comparison `Bool` to `i1` when
an explicit type handle is needed, device pointers to AS1, nonempty fixed `int` arrays and their AS1
pointers, exact raw `RWStructuredBuffer<int, DefaultLayout>`, and the canonical scalar-layout
element pointer produced by resource addressing. Signedness stays operation policy; it does not
create a second LLVM integer type hierarchy.

Slang access qualifiers are legality metadata and are intentionally absent from LLVM pointer type
identity. Read and read-write canonical pointer types therefore have separate source-cache entries
but share one representation-cache entry keyed by exact canonical pointee plus LLVM address space.
This preserves one AS1 pointer construction for the established copy and fixed-array kernels without
introducing structural equivalence between Slang types. The raw resource remains the provider's
dedicated `{ i32 addrspace(1)*, i64 }` launch value; the host does not flatten it.

Adding a type in a future slice now requires one canonical linked-IR classifier with its producer
and allowed uses, one explicit representation/address-space mapping in the type context, the
provider constructor only if V3 cannot already express it, and positive/adjacent-negative cache and
integration evidence. Function declaration, constants, phis, and every existing value path no
longer own or thread an `i32` handle, and the old scalar/pointer/array/resource singleton maps have
been removed.

The final Release NVVM prefix passes 193/193, including every real direct/NVRTC differential PTX,
matching-toolkit `ptxas`, and RTX 5090 runtime lane. Debug preservation passes 10/10. Slice 29 does
not change the provider ABI, exports, LLVM construction, accepted linked-IR subset, or observable
PTX/runtime contract.

### Slice 30 table-driven scalar evidence

Slice 30 changes no production behavior. The fake provider now records every V3 unary, binary, and
comparison callback as one `FakeNVVMBuilderScalarOperation`: a family and wire operation, ordered
operands, result handle, and insertion block. Frozen V2 callbacks remain as thin adapters into that
same stream, and failure injection is keyed by family and operation rather than stored in one field
per opcode. Dedicated atomic, resource, pointer, and array records stay separate because their
operand and result contracts are not scalar-family operations.

The eleven post-control-flow scalar operations with repeated evidence are declared in separate
unary, binary, and comparison descriptor arrays. A descriptor owns its Slang source, V3 operation,
kernel name, LLVM opcode classification, PTX evidence class, runtime operation, and diagnostic
name. Shared runners preserve independent provider-negotiation, invalid-operation, real-builder,
direct-topology, capability-gating, differential-PTX, `ptxas`, and runtime checks. Thin generated
wrappers retain all 88 operation/layer test names, so failures and capability-ledger references
remain granular. Add/subtract/signed-less-than continue in their established combined scalar CFG
fixtures, but use the same generic fake record.

Adding a future same-shaped V3 scalar operation therefore extends the appropriate descriptor family,
adds explicit runtime values and any operation-specific PTX assertion, and registers the applicable
thin evidence wrappers. It does not require fake storage, counters, operand lists, failure flags,
callback bodies, compile harnesses, assembler harnesses, or launch harnesses. The historical V2
layout cases remain explicit because their byte offsets, callback names, and predecessor prefixes
are frozen compatibility facts; new V3-only operations do not extend that matrix.

Across the five NVVM test/support files, physical source lines fall from 26,353 to 17,818 (down
8,535, or 32.4%), and nonblank lines fall from 24,476 to 16,613. The exact sorted 193-name set keeps
SHA-256 `1f35f717b93e1cb62c3f872e99b819386ab9c5474b203256e58ee1bdb41c97b7`.
Release passes 193/193, including every real PTX, `ptxas`, and RTX 5090 runtime lane; Debug
preservation passes 10/10. No production source, provider ABI, export, accepted IR shape, or
environment gate changes.

### Slice 31 exact scalar float32 addition

Slice 31 admits one exact second scalar family: raw CUDA kernels may receive scalar `float`
parameters and a read or read-write device `Ptr<float>`, add two available parameter values, and
store the result through the pointer. The final linked graph contains canonical `Float`, one AS1
float pointer, one `kIROp_Add`, and one four-byte-aligned store. Float loads remain outside the
accepted graph, so the supported source does not rely on constants, casts, helper calls, phis, or
another producer that would broaden this contract.

The append-only V3 provider table adds a `getFloatingPointType` constructor and a generic
`emitFloatingBinary` dispatcher. Feature bit `SCALAR_FLOAT32_ADD` requires both callbacks and the
complete prefix. The x64 table grows from the exact 448-byte V3 core to 464 bytes. On x86 the old
core's terminal-callback minimum remains 268 bytes, the new complete-prefix minimum is 276 bytes,
and structure padding makes `sizeof` 280 bytes. An old exact V3 core remains valid when it does not
advertise the feature; partial or null advertised prefixes are malformed. V2 neither grows nor
synthesizes the feature.

The module-local type context remains the only Slang-type-to-provider-type owner. It recognizes
only canonical `Float` and exact AS1 float pointers, caches the LLVM `float` and pointer handles,
and preserves access qualifiers as legality rather than LLVM type identity. The provider accepts
only bit width 32 and emits one unflagged LLVM `fadd` for available, dominating, same-module,
same-function LLVM `float` operands at an unterminated insertion point. The established audited
NVVM-2.0 text bridge already represents that graph, so this slice adds no textual rewrite.

Direct NVVM and NVRTC expose parameter widths `[64, 32, 32]`, a token-safe `add.f32`, and one
global 32-bit store with no global load. Matching-toolkit `ptxas` accepts both outputs, and CUDA
runtime results agree for `1.5 + 2.25 = 3.75`, `-8 + 0.5 = -7.5`, and
`1024 + -256 = 768`. Float subtraction, multiplication, negation, comparison, constants, loads,
helpers, phis, casts, half/double, vectors, aggregates, resources, and atomics remain deterministic
boundaries.

The fake provider records the new instruction through the same family/operation/ordered-operands
stream introduced in Slice 30; it gains no per-operation state bundle or dedicated end-to-end
harness. The final Release NVVM prefix passes 201/201. Its exact sorted LF-terminated name set has
SHA-256 `73434ac732eccaf42c9fad54ad2956b13aa5e2371e9e2e72d5fbbc2aaaf6e2e2`.
Debug preservation passes 10/10.

### Slice 32 exact scalar float32 device loads

Slice 32 admits the direct float memory producer deliberately left outside Slice 31: a raw CUDA
kernel may load one canonical float32 value from an AS1 read or read-write `Ptr<float>` and feed it
to the established aligned float store. The final graph has two pointer parameters, one load, and
one store. The shared pointer validator proves that the canonical pointer pointee equals the load
result and that the pointer is available at the consumer; there is no cast, helper, offset,
aggregate, or reconstructed value.

This capability requires the existing `SCALAR_MEMORY` and `SCALAR_FLOAT32_ADD` feature bundle but
adds no provider feature, callback, table field, export, or text rewrite. The original generic load
callback already constructs a typed aligned load from its pointer's LLVM pointee. Consequently an
exact Slice 31 provider already implements the complete provider side of this graph, and V1/V2/V3
sizes and compatibility rules remain unchanged.

The fake provider now indexes generic load values and records each result's scalar kind from the
typed pointer record. Integer pointer, pointer-offset, array-element, and resource-element loads
remain integer; a float-pointer parameter produces a float load. Both integer and float fake value
validators consume that record, so evidence no longer assumes that every load is i32 and does not
add per-type load callbacks or storage bundles.

Verified LLVM and audited NVVM-2.0 text contain exactly one aligned `load float` and one aligned
`store float`, with no `fadd`. Direct NVVM and NVRTC expose `[64, 64]`, a global 32-bit load and
store, and no float add. Matching-toolkit CUDA 12.9 `ptxas` accepts both outputs; on the RTX 5090,
both routes copy `3.75`, `-7.5`, `0`, and `1024` exactly. Pointer offsets, arrays, resources,
local/shared/global storage, volatile/atomic loads, half/double, and aggregates remain outside the
accepted float-load subset.

The final Release NVVM prefix passes 207/207. Its exact sorted LF-terminated name set has SHA-256
`5e9c007c59d45c4db5bf9724e6b76c039455d342330f06b8aa68cd2e5eb2316b`. Debug preservation
passes 10/10.

### Slice 33 exact scalar float32 subtraction

Slice 33 adds exact raw scalar `float` subtraction. Canonical float parameters feed one
`kIROp_Sub`, whose result is stored through the established AS1 float pointer. Preflight selects
integer versus floating semantics from the canonical result type, and emission routes ADD and
SUBTRACT through the same floating-binary facade with ordered operands.

V3 adds feature bit 21 and floating operation 1 but no table member: x64/x86 remain 464/280 bytes.
Exact Slice 31/32 feature sets continue to support addition and loads without subtraction. A
provider advertising subtraction must expose the already-complete float prefix; the facade checks
the requested operation's feature before dispatch. The provider applies its established
ownership/type/availability/dominance/insertion validation and emits one unflagged `CreateFSub`.

Verified LLVM and audited NVVM-2.0 text contain one `fsub float`, aligned store, kernel metadata,
no `fadd`, and no fast flags. NVVM and NVRTC expose `[64, 32, 32]`, `sub.f32`, a global 32-bit
store, and no global load/add. CUDA 12.9 `ptxas` accepts both. On the RTX 5090, both routes produce
`7.5`, `-8.5`, and `1280` for exact finite cases. Constants, helpers, phis, multiply/divide,
negation, comparisons, half/double, aggregates, and fast/constrained FP remain outside this slice.

The final Release NVVM prefix passes 214/214. Its exact sorted LF-terminated name set has SHA-256
`6ba1df40ff963723a866c61cbf8518aba7596e23213d5743015397547c90af9d`. Debug preservation passes
10/10.

### Slice 34 exact scalar float32 multiplication and scalable evidence

Slice 34 adds exact raw scalar `float` multiplication without returning to per-operation provider
wrappers or copied test harnesses. Canonical float parameters feed one `kIROp_Mul` and an aligned
store through the established AS1 float pointer. The same opcode with canonical signed-i32 type
continues to use the integer-binary family and its independent feature.

One emitter-owned `NVVMFloat32BinaryInfo` mapping now supplies the semantic feature, stable wire
operation, and diagnostic for ADD, SUBTRACT, and MULTIPLY. Both capability collection and emission
consume that mapping, so adding an accepted opcode cannot silently update one side without the
other. Feature 22 and floating operation 2 reuse the generic callback and do not grow the 464-byte
x64 or 280-byte x86 V3 table. Exact Slice 33 feature sets remain valid, while advertising
MULTIPLY requires the already-complete float prefix. The real provider applies the common
ownership/type/availability/dominance/insertion contract and emits one unflagged `CreateFMul`.

The three same-shape operations are also described once by `NVVMFloat32BinaryTestCase`: feature,
wire operation, source, kernel name, LLVM token, diagnostic label, and exact runtime cases.
Layer-specific builder, direct-topology, capability, differential-PTX, `ptxas`, and runtime runners
consume those facts behind thin registered wrappers. Existing ADD/SUBTRACT names and assertions
remain intact. Across the five test/support files measured by the scalability work, physical lines
fall from 19,608 to 19,512 while seven MULTIPLY names are added.

Verified LLVM and audited NVVM-2.0 text contain exactly one `fmul float`, one aligned store, kernel
metadata, no other floating-binary opcode, and no fast flags. Direct NVVM and NVRTC expose
`[64, 32, 32]`, token-safe `mul.f32`, one global 32-bit store, and no global load/add/sub. CUDA 12.9
`ptxas` accepts both outputs. On the RTX 5090, both routes produce `3`, `-4`, and `-256` for exactly
representable finite inputs. The former float-multiply negative fixture now advances to its next
honest unsupported producer, `castFloatToInt`.

The final Release NVVM prefix passes 221/221. Its exact sorted LF-terminated name set has SHA-256
`c24e6b4e82e289c2533444b0b0c0dab6cc44064a1df02d75a79928de94c2afa8`. Debug preservation passes
10/10.

### Slice 35 exact scalar float32 division

Slice 35 adds exact raw scalar `float` division by extending the Slice 34 closed mappings rather
than adding another provider or test shape. Canonical float parameters feed one `kIROp_Div` and an
aligned AS1 float store. Integer `kIROp_Div` deliberately remains E52017 `div`; its divide-by-zero
and signed-overflow policies are still unsettled and are not implied by this floating capability.

Feature 23 and floating operation 3 reuse the 464-byte x64/280-byte x86 V3 table and generic
callback. Exact Slice 34 providers remain valid. The host checks the independent feature before
dispatch, and the provider applies the established float ownership/availability/insertion contract
before unflagged `CreateFDiv`. No reciprocal approximation or fast-math flag is introduced.

One `NVVMFloat32BinaryTestCase` row and seven thin wrappers add negotiation, real-builder,
direct-topology, capability, differential-PTX, `ptxas`, and runtime evidence. The five measured
test/support files grow only 48 lines, from 19,512 to 19,560. LLVM and audited NVVM-2.0 text contain
exactly one `fdiv float` and no other floating-binary opcode. NVVM and NVRTC expose `[64, 32, 32]`,
a token-safe 32-bit division instruction, one global store, and no global load/add/sub/mul. CUDA
12.9 `ptxas` accepts both; the RTX 5090 produces `4`, `-16`, and `-4` for exactly representable
finite nonzero-denominator cases.

The final Release NVVM prefix passes 228/228. Its exact sorted LF-terminated name set has SHA-256
`99dec82e0909050b0dc909113dad988369dfe9b2666e5385faaec947c6c29bc7`. Debug preservation passes
10/10.

### Slice 36 exact scalar float32 negation and compatible V3 suffix

Slice 36 adds `*destination = -value` for a raw AS1 `Ptr<float>` and scalar float parameter.
Canonical Float `kIROp_Neg` now follows float validation and a new generic floating-unary family;
the same opcode with signed-i32 type retains the established integer-unary path. Unsigned and wide
entry parameters remain unsupported, and the older float-negate-plus-int-cast fixture now advances
to its next honest E52017 boundary, `castFloatToInt`.

Feature 24 advertises stable unary operation 0 through `emitFloatingUnary`, appended after the
floating-binary callback. V3 grows from 464 to 472 bytes on x64. On x86 the new four-byte function
pointer occupies Slice 35's tail padding, so both old and new `sizeof` values are 280 bytes. An exact
Slice 35 provider remains compatible because it cannot advertise feature 24; advertising the bit
requires the complete suffix and non-null float-type and unary callbacks. Unknown operations clear
their output and fail before dispatch.

The provider constructs one canonical unflagged LLVM `fneg float`. A direct probe found that CUDA
12.9 libNVVM's NVVM-2.0 textual reader rejects that LLVM-14 opcode with `parse expected instruction
opcode`. The audited compatibility writer therefore validates each semantic scalar float `fneg`
and rewrites only its exact printed line to legacy `fsub float -0.000000e+00, value`; semantic and
rewritten counts must agree. Generic LLVM assembly remains `fneg`, and no zero value or alternative
graph is introduced into Slang IR or the provider module. This slice claims exact finite ordinary
cases, not NaN payload, denormal, or constrained/fast-math behavior.

The former binary descriptor is now one five-row `NVVMFloat32ArithmeticTestCase` table with operand
count. Provider-kernel construction, direct topology, PTX evidence, `ptxas`, and runtime launch all
consume that descriptor, so unary support does not copy a second layer harness. The five measured
test/support files grow from 19,560 to 19,841 physical lines while adding the new ABI-prefix tests,
legacy-writer audit, fake family, and seven registered names.

Direct topology is `[FloatPointer, Float]` with operand parameter 1 and a store of its unary result.
NVVM and NVRTC agree on parameter widths `[64, 32]`, token-safe `neg.f32`, one global store, and no
global load or binary float instruction. CUDA 12.9 `ptxas` accepts both outputs. On the RTX 5090,
both routes produce `-1.5`, `8`, and `-1024` for the descriptor's exact finite cases.

The focused matrix passes 11/11 and the final Release NVVM prefix passes 235/235. Its exact sorted
LF-terminated name set has SHA-256
`2b79918702a9b21110af8251944e4428001a4ea69a2ff79b7a18e488cd13b4ba`. Debug preservation passes
10/10.

### Slice 37 exact scalar float32 ordered equality

Slice 37 adds `*destination = left == right ? 1 : 0` for two scalar Float parameters and a raw AS1
`Ptr<int>` destination. Normal lowering produces canonical Bool `kIROp_Eql`; canonical Float
operand types distinguish the new path from established signed-i32 equality because both forms
have the same Bool result. The comparison's `i1` output continues through the existing conditional
branch, zero/one constants, integer phi, and aligned i32 store without an alternate result
representation.

Feature 25 advertises stable floating-compare operation 0, `ORDERED_EQUAL`, through an
`emitFloatingCompare` callback appended after the floating-unary field. V3 grows from 472 to 480
bytes on x64 and from 280 to 288 bytes on x86. An exact Slice 36 provider remains compatible when
it does not advertise feature 25; advertising it requires the complete suffix plus the float-type
and compare callbacks. The facade rejects unknown operations before dispatch and clears every
failed output.

The provider applies the established ownership, insertion, availability, dominance, function, and
exact LLVM-float checks before emitting one unflagged `CreateFCmpOEQ`. Both generic LLVM and
negotiated NVVM-2.0 text contain exactly one `fcmp oeq float`; unlike LLVM 14 `fneg`, this opcode
needs no legacy text conversion. Orderedness makes the source-language NaN rule explicit: equality
is false when either operand is NaN.

The first `NVVMFloat32ComparisonTestCase` row owns source, feature, operation, text, PTX, and runtime
data. It shares a Boolean-result-to-i32 provider consumer with integer comparisons and the generic
float `ptxas` runner, but retains a separate descriptor family because Float argument ABI and
runtime cases differ from integer comparison data. The five measured test/support files grow from
19,841 to 20,503 physical lines for the new family scaffolding; another floating predicate needs a
row and thin wrappers rather than another layer harness.

Direct topology has parameter kinds `[Pointer, Float, Float]`; parameters 1 and 2 feed one floating
comparison whose Bool result controls the established four-block constant/phi/store graph. NVVM
and NVRTC agree on parameter widths `[64, 32, 32]`, a token-safe float32 equality predicate, one
global i32 store, and no global load, float arithmetic, or integer comparison predicate. CUDA 12.9
`ptxas` accepts both outputs. On the RTX 5090, both routes return one for `3.75 == 3.75` and
`+0 == -0`, and zero for `-8 != 0.5` and quiet `NaN == NaN`.

The focused matrix passes 12/12 and the Release NVVM prefix passes 242/242. Its exact sorted
LF-terminated name set has SHA-256
`7bdb7df316f95767ad79c76e2f802dc08504dfd06fbdfd5208a9c0eafd4ca670`. Debug preservation passes
10/10.

### Slice 38 exact scalar float32 unordered inequality

Slice 38 adds `*destination = left != right ? 1 : 0` for the same two scalar Float parameters and
raw AS1 `Ptr<int>` destination. Normal lowering produces canonical Bool `kIROp_Neq`; the existing
closed floating-comparison classifier now maps that exact Float-operand shape to feature 26 and
operation 1, while canonical signed-i32 `kIROp_Neq` stays on the integer comparison path. The Bool
result continues through the established four-block zero/one, integer-phi, and aligned-store graph.

`UNORDERED_NOT_EQUAL` reuses Slice 37's `emitFloatingCompare` field, so the V3 table remains 480
bytes on x64 and 288 bytes on x86, with a 284-byte semantic suffix before x86 tail padding. A Slice
37 provider remains valid without feature 26. The facade maps each stable operation to its exact
feature before dispatch, and the provider applies the shared handle/type/ownership/availability/
dominance/insertion checks before unflagged `CreateFCmpUNE`. Generic LLVM and negotiated NVVM-2.0
text each contain exactly one `fcmp une float` and need no compatibility rewrite. Unorderedness is
intentional: `!=` is true when either operand is NaN and is the logical complement of ordered
equality.

The second `NVVMFloat32ComparisonTestCase` row owns the new source, feature, operation, text,
kernel, PTX, and runtime data. The equality-only provider, direct, differential-PTX, assembler, and
runtime bodies became descriptor-driven helpers with thin registered wrappers. Consequently, seven
new names add 185 physical lines across the five measured test/support files, from 20,503 to
20,688, versus the 662-line first-family cost in Slice 37. The production direct emitter also
combines equality and inequality validation/emission around the same bounded classifier, deleting
the duplicated signed-i32 inequality block.

Direct topology remains `[Pointer, Float, Float]`; parameters 1 and 2 feed one floating comparison
whose Bool result controls the existing consumer. NVVM and NVRTC agree on `[64, 32, 32]`, a
token-safe float32 equality/inequality predicate family, one global i32 store, and no global load,
float arithmetic, or integer predicate. CUDA 12.9 `ptxas` accepts both outputs. On the RTX 5090,
both routes return zero for `3.75 != 3.75` and `+0 != -0`, and one for `-8 != 0.5` and quiet
`NaN != NaN`.

The focused matrix passes 13/13 and the Release NVVM prefix passes 249/249. Its exact sorted
LF-terminated name set has SHA-256
`529af4d3eba39ba0aabd6ca881ca3ac66b5f30c5f272c75a54a3b5cdc15156ea`; removing the seven Slice
38 names reproduces Slice 37's count and hash exactly. Debug preservation passes 10/10.

### Slice 39 exact scalar float32 ordered greater-than

Slice 39 adds `*destination = left > right ? 1 : 0` for two scalar Float parameters and the raw
AS1 `Ptr<int>` destination. Canonical Bool `kIROp_Greater` with Float operands maps directly to
feature 27 and operation 2, `ORDERED_GREATER_THAN`; signed-i32 greater-than remains on its established
integer operation. No operand reversal or alternative less-than representation is introduced.

The operation reuses `emitFloatingCompare`, leaving V3 at 480 bytes on x64 and 288 bytes on x86.
The facade's comparison-family suffix check and operation-to-feature switch grow by one row. The
provider applies its existing validation before unflagged `CreateFCmpOGT`; generic LLVM and
negotiated NVVM-2.0 text each contain exactly one `fcmp ogt float`. Orderedness makes comparisons
with either quiet-NaN operand false.

The third comparison descriptor row drives every established layer. The feature-negotiation test
for post-Slice-37 comparison rows is now descriptor-driven too. Seven independently registered
names add only 60 physical lines across the five measured test/support files, from 20,688 to
20,748. Combining signed and floating `kIROp_Greater` around the closed classifier deletes the old
duplicated direct-emission block, so production direct-emitter code shrinks in this slice.

Direct topology remains `[Pointer, Float, Float]` and preserves original parameter order. NVVM and
NVRTC agree on `[64, 32, 32]`, a token-safe float32 relation predicate family, one global i32 store,
and no global load, float arithmetic, or integer predicate. CUDA 12.9 `ptxas` accepts both outputs.
On the RTX 5090, both routes return one for `3.75 > 1.5`, and zero for `-8 > 0.5`, `+0 > -0`, and
quiet `NaN > -1`.

The focused matrix passes 14/14 and the Release NVVM prefix passes 256/256. Its exact sorted
LF-terminated name set has SHA-256
`f8b9a58433982e2583a7310c3e2bc43c82767adee115d121a13147783a8a6fcf`; removing the seven Slice
39 names reproduces Slice 38's count and hash exactly. Debug preservation passes 10/10.

### Slice 40 exact scalar float32 ordered less-than-or-equal

Slice 40 adds `*destination = left <= right ? 1 : 0` for two scalar Float parameters and the raw
AS1 `Ptr<int>` destination. Canonical Bool `kIROp_Leq` with Float operands maps directly to feature
28 and operation 3, `ORDERED_LESS_EQUAL`; signed-i32 less-than-or-equal remains on its established
integer operation. No operand reversal, complement, or alternative comparison representation is
introduced.

The operation reuses `emitFloatingCompare`, leaving V3 at 480 bytes on x64 and 288 bytes on x86.
The facade adds one comparison-family suffix predicate and maps the operation to its independent
feature. The provider applies its existing validation before unflagged `CreateFCmpOLE`; generic
LLVM and negotiated NVVM-2.0 text each contain exactly one `fcmp ole float`. Orderedness makes the
result false when either operand is a quiet NaN.

The fourth comparison descriptor row drives every established layer. Seven independently
registered names add only 41 physical lines across the five measured test/support files, from
20,748 to 20,789. Combining signed and floating `kIROp_Leq` around the closed classifier also
removes the old duplicated direct-emission block.

Direct topology remains `[Pointer, Float, Float]` and preserves original parameter order. NVVM and
NVRTC agree on `[64, 32, 32]`, a token-safe float32 relation predicate family, one global i32 store,
and no global load, float arithmetic, or integer predicate. PTX may spell the result as direct
ordered less-equal or the complement of unordered greater-than; CUDA 12.9 `ptxas` accepts both
outputs. On the RTX 5090, both routes return one for `1.5 <= 3.75` and `+0 <= -0`, and zero for
`0.5 <= -8` and quiet `NaN <= 1`.

The focused matrix passes 14/14 and the Release NVVM prefix passes 263/263. Its exact sorted
LF-terminated name set has SHA-256
`f93467f3b27def96040db05fca0fec79c5e22a5010ae6a3226fab4d249d860a1`; removing the seven Slice
40 names reproduces Slice 39's count and hash exactly. Debug preservation passes 10/10.

### Slice 41 exact scalar float32 ordered greater-than-or-equal

Slice 41 adds `*destination = left >= right ? 1 : 0` for two scalar Float parameters and the raw
AS1 `Ptr<int>` destination. Canonical Bool `kIROp_Geq` with Float operands maps directly to feature
29 and operation 4, `ORDERED_GREATER_EQUAL`; signed-i32 greater-than-or-equal remains on its
established integer operation. No operand reversal, complement, or alternative comparison
representation is introduced.

The operation reuses `emitFloatingCompare`, leaving V3 at 480 bytes on x64 and 288 bytes on x86.
The facade adds one comparison-family suffix predicate and maps the operation to its independent
feature. The provider applies its existing validation before unflagged `CreateFCmpOGE`; generic
LLVM and negotiated NVVM-2.0 text each contain exactly one `fcmp oge float`. Orderedness makes the
result false when either operand is a quiet NaN.

The fifth comparison descriptor row drives every established layer. Seven independently registered
names add only 52 physical lines across the five measured test/support files, from 20,789 to 20,841.
Combining signed and floating `kIROp_Geq` around the closed classifier also removes the old
duplicated direct-emission block.

Direct topology remains `[Pointer, Float, Float]` and preserves original parameter order. NVVM and
NVRTC agree on `[64, 32, 32]`, a token-safe float32 relation predicate family, one global i32 store,
and no global load, float arithmetic, or integer predicate. PTX may spell the result as direct
ordered greater-equal or the complement of unordered less-than; CUDA 12.9 `ptxas` accepts both
outputs. On the RTX 5090, both routes return one for `3.75 >= 1.5` and `+0 >= -0`, and zero for
`-8 >= 0.5` and quiet `NaN >= -1`.

The focused matrix passes 14/14 and the Release NVVM prefix passes 270/270. Its exact sorted
LF-terminated name set has SHA-256
`5358536da56531d08b93bd3e2f55d25d3d8cc42a21e461b3a905b1425a1f1fc4`; removing the seven Slice
41 names reproduces Slice 40's count and hash exactly. Debug preservation passes 10/10.

### Slice 42 exact scalar float32 ordered less-than

Slice 42 adds `*destination = left < right ? 1 : 0` for two scalar Float parameters and the raw
AS1 `Ptr<int>` destination. Canonical Bool `kIROp_Less` with Float operands maps directly to
feature 30 and operation 5, `ORDERED_LESS_THAN`; signed-i32 less-than remains on its original
`SCALAR_CONTROL_FLOW` feature and integer operation. No operand reversal, complement, or
alternative comparison representation is introduced.

The operation reuses `emitFloatingCompare`, leaving V3 at 480 bytes on x64 and 288 bytes on x86.
The facade adds one comparison-family suffix predicate and maps the operation to its independent
feature. The provider applies its existing validation before unflagged `CreateFCmpOLT`; generic
LLVM and negotiated NVVM-2.0 text each contain exactly one `fcmp olt float`. Orderedness makes the
result false when either operand is a quiet NaN.

The sixth and final scalar comparison descriptor row drives every established layer. Seven
independently registered names add 50 physical lines across the five measured test/support files,
from 20,841 to 20,891. Combining signed and floating `kIROp_Less` around the closed classifier also
removes the original duplicated direct-emission block while retaining its integer feature mapping.

Direct topology remains `[Pointer, Float, Float]` and preserves original parameter order. NVVM and
NVRTC agree on `[64, 32, 32]`, a token-safe float32 relation predicate family, one global i32 store,
and no global load, float arithmetic, or integer predicate. PTX may spell the result as direct
ordered less-than or the complement of unordered greater-equal; CUDA 12.9 `ptxas` accepts both
outputs. On the RTX 5090, both routes return one for `1.5 < 3.75`, and zero for `0.5 < -8`,
`+0 < -0`, and quiet `NaN < 1`.

The focused matrix passes 14/14 and the Release NVVM prefix passes 277/277. Its exact sorted
LF-terminated name set has SHA-256
`a34a5cdb1532603a18290777a75fe23ea9407f5d294e1d9a1a739ea6b9187ae6`; removing the seven Slice
42 names reproduces Slice 41's count and hash exactly. Debug preservation passes 10/10.

### Slice 43 exact scalar float32 constants

Slice 43 adds `*destination = 1.5f` for a raw AS1 `Ptr<float>` destination. Canonical Float
`kIROp_FloatLit` stores its value in Slang's double-backed `IRFloatingPointValue`, so the direct
emitter rounds once to the semantic float32 value and transports the exact IEEE-754 payload
`0x3fc00000`. It does not serialize decimal text or route Float values through the frozen signed-i32
constant callback.

Feature 31, `SCALAR_FLOAT32_CONSTANT`, appends a generic
`getFloatingPointConstant(module, type, bitWidth, bitPattern, outValue)` callback. The facade and
provider accept only width 32 with zero high bits in this slice, validate the Float type's module
context, clear failed outputs, and construct `ConstantFP` from LLVM `APFloat`/`APInt`. The x64 V3
table grows from 480 to 488 bytes. On x86, the callback occupies the former tail padding: its
semantic minimum and complete table remain 288 bytes. Exact Slice 42 tables remain valid when they
do not advertise feature 31.

Direct preflight now treats only canonical scalar Float literals as executable constant operands,
requests feature 31, and materializes them on demand through the type-lowering cache. The fake
provider records one `FloatingPointConstant` node with width 32 and payload `0x3fc00000`; the store
consumes that node through the sole Float-pointer parameter. Signed-i32 literals continue to request
`SCALAR_SSA` and use V2 `getIntegerConstant` unchanged.

The new value-family infrastructure and seven independently registered evidence layers add 554
physical lines across the five measured test/support files, from 20,891 to 21,445. Generic LLVM and
negotiated NVVM-2.0 text each contain one `store float 1.500000e+00` and no synthetic arithmetic.
NVVM and NVRTC agree on the single `[64]` pointer parameter, one global 32-bit store, and no global
load, Float arithmetic, or predicate. CUDA 12.9 `ptxas` accepts both outputs, and both routes write
the exact float32 value `1.5` on the RTX 5090.

The focused matrix passes 14/14 and the Release NVVM prefix passes 284/284. Its exact sorted
LF-terminated name set has SHA-256
`3e78b6b3069dd0a12cbde4d78e4d804e5eeace161cdbf86d620262b5e9d9a72d`; removing the seven Slice
43 names reproduces Slice 42's count and hash exactly. Debug preservation passes 10/10.

### Slice 44 generic scalar phis and float32 SSA merging

Slice 44 adds a conditional merge of two Float entry parameters before an aligned AS1 Float store.
Slang's canonical representation is already a Float block parameter plus positional Float
arguments on its two actual predecessor branches. The direct backend now accepts that semantic
type alongside signed i32 instead of reconstructing a local variable or introducing another SSA
form.

Feature 32, `SCALAR_PHI`, appends generic
`emitPhi(module, targetBlock, type, outValue)` and
`addPhiIncoming(module, phi, value, predecessorBlock)` callbacks. The type handle makes this one
scalar family rather than one callback pair per scalar type. V3 grows from 488 to 504 bytes on x64
and from 288 to 296 bytes on x86. Exact Slice 43 tables remain valid without feature 32, while an
advertised feature requires both complete callbacks. Frozen V2 integer-phi callbacks and feature 6
remain unchanged.

The provider shares phi construction and incoming-edge validation between its V2 integer adapters
and generic V3 adapters. It preserves exact type/module/function ownership, insertion before the
first non-phi, complete CFG, one real predecessor edge, duplicate rejection, and incoming-value
dominance. Generic V3 accepts scalar Integer and Float types; the completed direct slice uses it for
Float only. The fake similarly records a typed `ScalarPhi`, while retaining its separate frozen-V2
integer record for compatibility evidence.

For the motivating source, direct topology is `[FloatPointer, Integer, Float, Float]`. One integer
condition controls four blocks, and one Float phi receives original parameters 2 and 3 from the
two actual predecessors before the store. Generic LLVM and negotiated NVVM-2.0 text each contain
exactly one `phi float`. NVVM and NVRTC agree on `[64, 32, 32, 32]`, one global 32-bit store, and no
global load, Float arithmetic, or Float predicate. CUDA 12.9 `ptxas` accepts both outputs. On the
RTX 5090 both routes select finite values and preserve the selected `-0.0` versus `+0.0` bit.

The generic phi/fake/runtime-family base plus seven independently registered evidence layers adds
709 physical lines across the five measured test/support files, from 21,445 to 22,154. The runtime
setup is factored into a typed callable harness for reuse by later Float type slices. The focused
matrix passes 14/14 and the Release NVVM prefix passes 291/291. Its exact sorted LF-terminated name
set has SHA-256 `c18462cd303630788566c59409f369ef57a46614652571a97663acf0ffb01690`;
removing the seven Slice 44 names reproduces Slice 43's count and hash exactly. Debug preservation
passes 10/10.

### Slice 45 generic scalar functions and float32 helper calls

Slice 45 adds a reachable `float addFloat32(float left, float right)` helper whose result reaches an
aligned AS1 Float store in the kernel. Slang's canonical direct-call closure, semantic helper
signature, positional call arguments, and return value remain the only representation; the backend
does not reconstruct a helper ABI or introduce type-specific call nodes.

Feature 33, `GENERIC_SCALAR_FUNCTIONS`, appends generic
`emitCall(module, callee, arguments, count, outValue)` and
`emitValueReturn(module, value)` callbacks. The callee function type and value handles carry the
scalar types, so one pair serves Integer and Float rather than growing per-type wrappers. V3 grows
from 504 to 520 bytes on x64 and from 296 to 304 bytes on x86. Exact Slice 44 tables remain valid
without feature 33, while an advertised feature requires floating-point type discovery and both
complete callbacks. Frozen V2 signed-i32 call/return callbacks and feature 3 remain unchanged.

The provider shares construction and validation between V2 integer adapters and generic V3
adapters. It preserves the same-module non-variadic callee, non-void scalar result, exact scalar
parameter/argument types, usable insertion-point operands, current-function return type, and
dominance checks. V2 requires Integer throughout; V3 accepts scalar Integer and Float, including
mixed signatures. The direct emitter chooses the path from the complete canonical helper signature
and asserts that signature was already preflighted.

The fake graph contains a Void kernel with `[FloatPointer, Float, Float]` and a Float helper with
`[Float, Float]`. Original kernel parameters 1 and 2 feed one typed Float call, the helper's
parameters feed one Float addition and generic valued return, and the call result feeds the sole
store. Generic LLVM and negotiated NVVM-2.0 text each contain one Float helper definition, one
`call float`, one `ret float`, and one `fadd float`.

NVVM and NVRTC agree on `[64, 32, 32]`, one Float addition, one global 32-bit store, and no global
load or Float predicate. CUDA 12.9 `ptxas` accepts both outputs. On the RTX 5090 both routes agree
for finite additions and preserve the exact results of `-0.0 + -0.0` and `+0.0 + -0.0`. The
previous floating-sine boundary now passes its Float helper signature and stops later at the still
unsupported `castFloatToInt`; every other unsupported-matrix boundary remains stable.

The generic call/fake/result/return base plus seven independently registered evidence layers adds
683 physical lines across the five measured test/support files, from 22,154 to 22,837. The focused
matrix passes 14/14 and the Release NVVM prefix passes 298/298. Its exact sorted LF-terminated name
set has SHA-256 `71658634899192b09f2d12461c25a5efb9d85c3c4f2db7c285ba35ef35d44066`;
removing the seven Slice 45 names reproduces Slice 44's count and hash exactly. Debug preservation
passes 10/10.

### Slice 46 wave lane index and canonical unsigned-i32 transport

Slice 46 adds a kernel that stores `WaveGetLaneIndex()` to `destination[laneIndex]` through a
canonical `Ptr<uint, ReadWrite, Device>`. CUDA target selection produces one retained
`Func(UInt)` helper with no parameters whose sole block terminates in exact
`GenericAsm("_getLaneId()")`; the kernel calls that helper and uses the result for both its pointer
offset and stored value. Direct lowering recognizes that already-selected semantic shape. It does
not search source names, reconstruct syntax, or expose arbitrary assembly text to the provider.

Canonical Slang `Int` and `UInt` remain distinct semantic types but now share LLVM's signless `i32`
for entry/helper transport, exact calls/returns, device pointers, pointer offsets, and stores.
Signedness-sensitive arithmetic, comparisons, atomics, and fixed-array indexing retain their
existing signed-only classifiers. UInt constants also remain unsupported. Consequently thirteen
negative fixtures advance from an obsolete UInt entry/value rejection to their first real
boundary: the UInt pointer-offset case reaches its constant, while unsigned multiply, bitwise,
negate, atomic, and comparison cases reach their signed-only operation diagnostics.

Feature 34, `WAVE_LANE_INDEX`, appends one generic
`emitIntrinsic(module, operation, arguments, count, outValue)` callback and stable operation 0.
The x64 V3 table grows from 520 to 528 bytes and x86 from 304 to 308 bytes. Exact Slice 45 and
earlier prefixes remain valid when they do not advertise feature 34; an advertised feature
requires the complete callback. The wrapper rejects unknown operations, clears failed outputs,
and leaves V2 unchanged.

The LLVM 14 provider maps the operation to
`llvm.nvvm.read.ptx.sreg.laneid`, after validating the zero-argument insertion state. LLVM 14
prints six optimization attributes on that declaration which the LLVM-7-era NVVM 2.0 parser does
not recognize. The audited legacy writer first verifies the exact semantic intrinsic declaration
and attribute set, then narrows only that attribute group to `nounwind readnone`; exact semantic
and rewritten counts must agree. Generic LLVM and negotiated NVVM text each contain one lane-id
call, one UInt helper call/return, and one store.

NVVM and NVRTC agree on the `[64]` launch ABI, one global 32-bit store, and no global load. Direct
NVVM selects PTX `%laneid`; NVRTC flattens the three-dimensional thread ID and masks it with `31`.
CUDA 12.9 `ptxas` accepts both, and one 32-thread RTX 5090 warp writes exactly 0 through 31 through
both routes.

The generic intrinsic/fake/runtime-family base plus seven independently registered evidence layers
adds 531 physical lines across the five measured test/support files, from 22,837 to 23,368. The
focused Slice 45/46 matrix passes 14/14 and the Release NVVM prefix passes 305/305. Its exact sorted
LF-terminated name set has SHA-256
`a5d99d25f4218d69bf938e171083e49c3826150873a58506c42e2b8bcbf98dbb`; removing the seven
Slice 46 names reproduces Slice 45's count and hash exactly. Debug preservation passes 10/10.

## CUDA Pass Ownership Audit

As the first Slang-to-NVVM emitter expands beyond empty compute, each current CUDA-specific
behavior must be placed in one of four groups: shared CUDA semantics, CUDA-C++ representation,
NVVM representation, or obsolete after the split. Important initial audit items include:

| Current behavior | Audit question |
| --- | --- |
| OptiX entry-point uniform collection | Defer entirely, or identify a compute semantic hidden by the OptiX-specific branch? |
| CUDA entry-point preservation and builtin lowering | Which invariants are common to both routes? |
| Global-constant inlining | Is this only an NVRTC dynamic-initialization workaround? |
| CUDA varying-parameter legalization | Which part defines the CUDA launch ABI and which part only emits C++ parameters? |
| Parameter-copy and const-reference transforms | Can NVVM preserve values/SSA instead of reconstructing C++ reference semantics? |
| Phi elimination | Keep SSA for NVVM unless a measured libNVVM constraint requires otherwise. |
| Resource legalization disabled for CUDA source | Slice 26 defines exact raw `RWStructuredBuffer<int>` launch storage; conventional globals and every other resource shape remain open. |

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
| 3 | Types and memory | pointer addressing, vectors, matrices, aggregates, layout, generic/global/shared/local address spaces |
| 4 | Core CUDA execution | thread/block IDs, barriers, atomics, shared memory, memory ordering |
| 5 | Numeric library policy | half/bfloat/fp8, transcendental math, libdevice, fast/precise/denormal modes |
| 6 | Resource ABI | buffers, textures, samplers, surface operations, bindless/resource handles |
| 7 | Slang language lowering | generics, interfaces, witness tables, specialization, autodiff |
| 8 | Advanced NVIDIA paths | waves, cooperative features, OptiX, debug metadata, RDC, LTO, dynamic parallelism |

Source-emission tests whose contract is specifically CUDA C++ spelling, macro expansion, header
shape, or prelude text stay assigned to NVRTC. Add a semantic counterpart before using such a test
as evidence for NVVM.

The checked-in [NVVM capability ledger](nvvm-backend-capability-ledger.md) records at least:

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
8. complete scalar program structure with executable constants, phis, and loops;
9. direct calls and the non-void helper ABI;
10. signed device-pointer element offsets;
11. fixed device-array element addressing;
12. signed-i32 multiplication;
13. signed-i32 bitwise AND;
14. signed-i32 bitwise OR;
15. signed-i32 bitwise XOR;
16. signed-i32 bitwise NOT;
17. signed-i32 arithmetic negation;
18. libdevice and floating-point policy;
19. relaxed global signed-i32 atomic add;
20. an isolated LLVM 7.0.1 native-bitcode feasibility experiment;
21. signed-i32 equality;
22. signed-i32 inequality;
23. signed-i32 greater-than;
24. signed-i32 less-than-or-equal;
25. signed-i32 greater-than-or-equal;
26. exact raw `RWStructuredBuffer<int>` storage;
27. behavior-preserving decomposition of the NVVM test harness;
28. a generic V3 provider ABI and feature-set negotiation, with frozen V2 fallback;
29. centralized Slang-IR-to-NVVM type legalization and provider-type caching;
30. table-driven consolidation of the scalar provider and end-to-end test matrix;
31. exact scalar float32 addition through the generic V3 provider family;
32. exact scalar float32 device-pointer loads;
33. exact scalar float32 subtraction through the generic floating-binary family;
34. exact scalar float32 multiplication plus descriptor-driven floating-binary evidence;
35. exact scalar float32 division through the generic floating-binary family;
36. exact scalar float32 negation through an append-only generic floating-unary V3 suffix;
37. exact scalar float32 ordered equality through an append-only generic floating-compare V3
    suffix;
38. exact scalar float32 unordered inequality through the generic floating-compare family;
39. exact scalar float32 ordered greater-than through the generic floating-compare family;
40. exact scalar float32 ordered less-than-or-equal through the generic floating-compare family;
41. exact scalar float32 ordered greater-than-or-equal through the generic floating-compare
    family;
42. exact scalar float32 ordered less-than through the generic floating-compare family;
43. exact scalar float32 constants through an append-only exact-bit V3 callback;
44. generic scalar phis and exact float32 block-parameter SSA merging;
45. generic scalar functions and exact float32 helper parameters, calls, results, and returns;
46. exact wave lane index through a generic intrinsic family and canonical unsigned-i32 transport;
47. remaining wave operations and other advanced capabilities, then production-readiness
    evaluation.

Slice 3b hardens the builder boundary between items 3 and 4 with versioned verifier diagnostics and
the reverse LLVM load-order proof; it deliberately adds none of item 4's scalar or pointer surface.
Slice 4 completes item 4 for the two scalar-memory reference kernels while deliberately adding no
Slang IR traversal or experimental routing from item 5.
Slice 5 completes item 5 by freezing the target option, preserving the NVRTC and pass-through
routes, reserving an honest NVVM dispatch boundary, and proving builder bitcode can reach the
session-registered compiler. It deliberately leaves the Slang-IR-to-NVVM producer to item 6.
Slice 6 completes item 6 for an empty, zero-parameter compute entry point. It establishes canonical
linked-IR legality, verified builder bitcode, the internal LLVM-kernel artifact, and real
libNVVM/`ptxas` handoff while leaving the kernel ABI and scalar control flow to Slice 7.
Slice 7 completes the first raw CUDA scalar parameter ABI and a deliberately phi-free part of item
7. It adds signed `i32` arithmetic/comparison and acyclic branches while retaining executable
constants, block parameters/branch arguments, phis, loops, calls, and richer types as the next
program-structure boundary. Slice 8 completes constants, phis, and finite loops. Slice 9 completes
the separately demonstrable direct-call/non-void-helper ABI. Slice 10 then proves signed-i32 element
offsetting on the existing device pointer ABI. Slice 11 takes the next bounded part of the aggregate
roadmap: exact fixed signed-i32 arrays behind device entry-point pointers and their canonical
`IRGetElementPtr`, without claiming array values, other aggregates, shared memory, or additional
address spaces. Slice 12 then completes the smallest remaining canonical integer-expression
boundary: exact two-operand signed-i32 multiplication, without widening the frozen ADD/SUB ABI or
claiming other integer, floating-point, vector, or matrix operations. Slice 13 applies the same
dedicated-operation pattern to exact signed-i32 bitwise AND, and Slice 14 applies it to exact
signed-i32 bitwise OR. Slice 15 applies the same pattern to exact signed-i32 bitwise XOR. Slice 16
then adds exact signed-i32 bitwise NOT through a dedicated unary operation and a shared
per-value integer-validation rule. Slice 17 completes the next bounded canonical scalar operation:
wrapping signed-i32 arithmetic negation. Slice 18 then freezes the explicit downstream libdevice
demand, same-toolkit linking, and fp32 option policy without claiming direct Slang f32 lowering.
Slice 19 takes one bounded half of the former atomics-and-waves roadmap entry: exact canonical
Relaxed signed-i32 atomic add through the established read-write device pointer ABI. Other atomic
operations, orders, types, and address spaces remain later boundaries, and waves move to the
advanced-capability track because they require independent lane, convergence, and intrinsic
contracts. A future bounded shift candidate must first settle the currently
inconsistent negative/oversized shift-count policy across AST folding, SCCP, LLVM, and PTX before
promoting exact signed-i32 left shift; division, remainder, and the other richer scalar policies
remain separate decisions.

Slices 21 through 25 complete the exact signed-i32 comparison family, and Slice 26 begins the
resource bucket with the measured raw `RWStructuredBuffer<int>` launch ABI. Those deliberately
narrow vertical slices established strong ownership, no-mutation, differential-PTX, assembler,
and runtime contracts, but their one-provider-callback and bespoke-test pattern is not the
steady-state architecture. Before another semantic capability is added, Slices 27 through 30 form
a scalability transition. Slice 27 separates fake-provider, builder-ABI, direct-emitter,
downstream-compiler, and real integration/runtime tests without changing their names or behavior.
Slice 28 freezes V2 and introduces a V3 provider surface whose Slang-owned operation enums and
feature set grow by semantic family rather than by slice prefix. The host prefers V3, falls back to
V2 only when the V3 export is absent, and treats a present but malformed V3 provider as an error.
Slice 29 makes one type-legalization/cache context own every mapping from canonical Slang IR types
to provider handles while preserving the exact Slice 26 subset. Slice 30 replaces duplicated
scalar fake state and test bodies with recorded generic operations and table-driven cases. New
types, resources, atomics, and waves resume only after this transition passes the established
focused, preservation, `ptxas`, and runtime evidence.

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

On 2026-08-26, Slice 4 built the standalone provider in Release against pinned LLVM 14.0.6 and the
host/unit tests in Debug. With the provider selected, the complete NVVM unit prefix passed 32/32,
including scalar ABI negotiation, invalid-operation/no-mutation coverage, verified reference
kernels, NVRTC-versus-libNVVM PTX comparison, and `ptxas` acceptance. The typed AS1 kernels had the
expected `[64, 32]` and `[64, 64]` parameter widths and global-memory operations. CUDA 12.9
`ptxas` 12.9.86 accepted both kernels from both routes for `sm_75`.

With the provider absent, four fake-only tests passed and ten real-provider tests ignored. An
explicit broken provider path failed a selected real test instead of skipping it. The isolated
two-LLVM coexistence test and the established shared-library, downstream-version, and NVRTC CUDA
sampler regressions also passed. PE inspection found exactly the V1 and V2 getters and no LLVM DLL
dependency. Ordinary PTX compilation remains routed through NVRTC.

Later on 2026-08-26, Slice 5 built the Debug `slangc` and unit-test targets and passed the focused
parser, method resolver, `linkWithOptions` routing/hash, invalid-method, explicit-NVVM diagnostic,
default/explicit-NVRTC, and raw-pass-through precedence tests. The affected component-hash and
session-digest regressions also passed. With the LLVM 14 provider selected, the new
registered-compiler handoff passed and the complete NVVM unit prefix passed 33/33. Default PTX
remained transition-driven NVRTC, explicit NVRTC bypassed a mutable transition override, and
explicit NVVM reached E52014 without fallback. No ordinary Slang program was claimed as
NVVM-lowered at that boundary.

Later on 2026-08-26, Slice 6 built the Debug host and the Release LLVM 14.0.6 provider. The complete
NVVM unit prefix passed 40/40, including ordinary-Slang fake and real direct-route compilation,
builder-aware cache hashing, unsupported-IR rejection, builder-verifier diagnostic propagation,
unavailable-provider behavior, and the established provider/compiler regressions. The real route
emitted `computeMain` for `cuda_sm_7_0`, and CUDA 12.9 `ptxas` accepted the resulting PTX. The
provider-independent file test reached E52017 on the retained barrier `call`. Default and explicit
NVRTC behavior and true NVRTC pass-through remained unchanged.

Later on 2026-08-26, Slice 7 rebuilt the Debug host and Release LLVM 14.0.6 provider and exercised
three ordinary-Slang kernels through both explicit PTX routes: a direct scalar store, a scalar
load/store copy, and an `if` choosing signed `i32` addition or subtraction. The direct route's fake
test checked exact parameter order, value producers, memory operands, comparison operands, and
branch targets. Real NVVM and NVRTC PTX agreed on parameter widths `[64, 32]`, `[64, 64]`, and
`[64, 32, 32]`, on the expected global-memory operations, and on signed comparison/addition for
the conditional kernel. CUDA 12.9 `ptxas` accepted all three kernels from both routes for `sm_75`.

The CUDA-driver differential launched both versions of all three kernels on a local device with
compute capability 7.0 or newer. Both routes stored `37`, copied `-17`, and produced `7`, `4`, `0`,
and `-1` for less-than, greater-than, equal, and negative signed-comparison inputs. Focused
rejection tests also proved that an old exact scalar-memory provider does not expose the appended
control-flow calls, malformed partial tables are rejected, cross-module, cross-function,
non-dominating, and post-terminator operands are rejected before mutation, only the selected raw
CUDA kernel survives linking, and a conventional parameterized compute entry reaches E52017 before
builder or libNVVM program creation.

Later on 2026-08-26, Slice 8 rebuilt the Debug host and Release LLVM 14.0.6 provider after pinned
clang-format 17 and passed the complete `slang-unit-test-tool/nvvm` prefix, 55/55. The exact fake
graphs covered add-one, the two-arm merge phi, and the canonical six-block `sumToLimit` loop with
constants `0` and `1`, two header phis, four incoming pairs, a distinct continue block, and a
distinct exit-to-break edge. The real provider emitted and verified the corresponding two-phi LLVM
loop. Its rejection test covered signed representability, incomplete CFGs, foreign handles,
non-predecessors, duplicates, and same-function non-dominating edge values without leaving partial
IR.

The same six ordinary-Slang scalar/SSA kernels compiled through NVRTC and direct NVVM with matching
raw parameter widths and global-memory semantics. CUDA 12.9.86 `ptxas` accepted every kernel from
both routes. On an RTX 5090 (compute capability 12.0, driver 610.62), both routes agreed on add-one
for positive and negative inputs, every less/greater/equal merge case, and `sumToLimit(0/5/7)` =
`0/10/21`. Post-format preservation runs also passed CUDA-option parsing (1/1), routing and hashing
(2/2), the unsupported barrier-call file (1/1), default/explicit NVRTC sampler coverage (3/3), true
NVRTC pass-through (2/2), and CUDA runtime dispatch (1/1). The provider still exports only the V1
and V2 getters and has no process-visible LLVM DLL dependency.

Later on 2026-08-26, Slice 9 added the transitive `increment`/`incrementTwice` helper graph without a
source-level `noinline` attribute. The exact fake graph contained three function definitions, four
direct calls, two valued helper returns, one kernel void return, and only one kernel annotation. An
unreachable helper containing multiplication was pruned before preflight. The LLVM 14 provider
verified the corresponding `define i32`, `call i32`, and `ret i32` module, and its negative tests
rejected missing insertion points, invalid callees, arity/type/module/function/dominance errors, and
post-terminator calls/returns without partial mutation.

The ordinary source compiled through NVRTC and direct NVVM with matching `[64, 32]` kernel parameter
widths and global-store semantics. CUDA 12.9.86 `ptxas` accepted both outputs, and both routes
produced `13` for input `5` and `-1` for input `-2` on the RTX 5090. The post-format complete NVVM
prefix passed 60/60 alongside the established routing, NVRTC, pass-through, unsupported-IR, and
CUDA-runtime regressions. The provider continued to export only its V1/V2 getters with no
process-visible LLVM DLL dependency.

On 2026-08-27, Slice 10's Release LLVM 14.0.6 provider build passed, and the final formatted
`slang-unit-test-tool/nvvm` prefix passed 68/68. The verified provider
fixture emitted two ordinary address-space-1 `getelementptr i32` values without `inbounds`. The fake
direct-route graph proved that both offsets use the exact shared signed-i32 parameter, that the
source offset feeds the load, and that the destination offset feeds the store. Negotiation kept an
exact Slice 9 prefix usable and rejected partial or incomplete pointer-arithmetic prefixes; invalid
provider operations failed before mutation. The unsigned source offset remained the deterministic
signed-i32 unsupported boundary.

Direct NVVM and NVRTC exposed matching `[64, 64, 32]` kernel parameter widths and global-memory
semantics. Both PTX lanes passed CUDA 12.9.86 `ptxas`, and both runtime lanes copied the intended
element for a positive allocation-base index and for index `-1` from interior base pointers while
preserving neighboring sentinels. The preservation runs passed CUDA-option parsing (1/1), routing
and hashing (2/2), the unsupported-IR file (1/1), default/explicit NVRTC sampler coverage (3/3),
true NVRTC pass-through (2/2), and CUDA runtime dispatch (1/1).

Later on 2026-08-27, Slice 11 built the Release LLVM 14.0.6 provider and Debug host, and the final
`slang-unit-test-tool/nvvm` prefix passed 76/76. The exact fake graph proved one shared provider
`[4 x i32]` type, two address-space-1 array-pointer parameters, one signed-i32 index parameter, and
the exact base/index/result/load/store topology. The verified provider fixture emitted the two
ordinary, non-`inbounds` array element GEPs. Negotiation retained the exact Slice 10 prefix for its
published programs, rejected partial or incomplete two-operation array prefixes, and sanitized
failed outputs. Invalid provider operations were rejected before mutation.

Direct NVVM and NVRTC exposed matching `[64, 64, 32]` parameter widths and each showed the expected
entry-scoped global i32 load/store behavior; this is a semantic comparison, not a PTX-text equality
claim. CUDA 12.9 `ptxas` accepted both outputs. On the RTX 5090, both runtime lanes copied array
indices `0` and `3` while preserving every neighboring sentinel. Post-format preservation passed
CUDA-option parsing (1/1), routing and hashing (2/2), the unsupported-IR file (1/1),
default/explicit NVRTC sampler coverage (3/3), true NVRTC pass-through (2/2), and CUDA runtime
dispatch (1/1). Binary inspection found only the frozen V1 and V2 provider getters in the export
table, a normal `KERNEL32.dll` dependency plus delay-loaded `SHELL32.dll` and `ole32.dll`, and no
LLVM DLL dependency.

Later on 2026-08-27, Slice 12 built the Release LLVM 14.0.6 provider and Debug host. The verified
provider fixture emitted exactly one `mul i32` whose result feeds the established address-space-1
store, and the fake direct-route graph proved that the two scalar parameters are the exact left and
right operands and that the multiply result is the stored value. Negotiation retained the exact
Slice 11 prefix for fixed-array programs, rejected a partial or null multiply suffix, accepted
future-larger tables, and sanitized failed outputs. Invalid module, type, function, availability,
dominance, insertion-point, and output shapes were rejected before mutation.

Direct NVVM and NVRTC exposed matching `[64, 32, 32]` parameter widths and each showed a 32-bit
integer multiply plus the expected global i32 store, observed locally as `mul.lo.s32` and
`st.global.u32`; the test makes a semantic comparison rather than a PTX-text equality claim. CUDA
12.9 `ptxas` accepted both outputs. On the RTX 5090, both runtime lanes produced `42`, `-42`, and
`0` for positive, negative, and zero cases. After correcting the prior Slice 11 test to distinguish
its frozen prefix from the now-larger full table, the complete focused NVVM prefix passed 84/84.

Later on 2026-08-27, Slice 13 built the Release LLVM 14.0.6 provider and Debug host. The verified
provider fixture emitted exactly one `and i32` whose result feeds the established address-space-1
store, and the fake direct-route graph proved that the two scalar parameters are the exact left and
right operands and that the bitwise-AND result is the stored value. Negotiation retained the exact
272-byte Slice 12 prefix for multiplication programs, rejected every partial size from 273 through
279 bytes and a null complete operation, accepted and clamped future-larger tables, and sanitized
failed outputs. Invalid module, output, integer type, function, availability, dominance, and
insertion-point shapes were rejected before mutation.

Direct NVVM and NVRTC exposed matching `[64, 32, 32]` parameter widths and each showed `and.b32`
plus the expected global i32 store. CUDA 12.9 `ptxas` accepted both outputs. On the RTX 5090, both
runtime lanes produced `0x18` for `0x5a & 0x3c`, `0x12345678` for `-1 & 0x12345678`, `-4` for
`-2 & -4`, and `0` for `0 & -1`. The final complete focused NVVM prefix passed 92/92.

Later on 2026-08-27, Slice 14 built the Release LLVM 14.0.6 provider and Debug host. The verified
provider fixture emitted exactly one `or i32` whose result feeds the established address-space-1
store, and the fake direct-route graph proved that the two scalar parameters are the exact left and
right operands and that the bitwise-OR result is the stored value. Negotiation retained the exact
280-byte Slice 13 prefix for bitwise-AND programs, rejected every partial size from 281 through 287
bytes and a null complete operation, accepted and clamped future-larger tables, and sanitized
failed outputs. Invalid module, output, integer type, function, availability, dominance, and
insertion-point shapes were rejected before mutation.

Direct NVVM and NVRTC exposed matching `[64, 32, 32]` parameter widths and each showed token-safe
`or.b32` plus the expected global i32 store. CUDA 12.9 `ptxas` accepted both outputs. On the RTX
5090, both runtime lanes produced `0x7e` for `0x5a | 0x3c`, `-13` for `-16 | 3`, `-1` for
`0 | -1`, and `0x5f5f5f5f` for `0x55555555 | 0x0f0f0f0f`. After correcting the prior Slice 13
test to distinguish its frozen prefix from the now-larger full table, the complete focused NVVM
prefix passed 100/100.

Later on 2026-08-27, Slice 15 built the Release LLVM 14.0.6 provider and Debug host. The verified
provider fixture emitted exactly one `xor i32` whose result feeds the established address-space-1
store, and the fake direct-route graph proved that the two scalar parameters are the exact left and
right operands and that the bitwise-XOR result is the stored value. Negotiation retained the exact
288-byte Slice 14 prefix for bitwise-OR programs, rejected every partial size from 289 through 295
bytes and a null complete operation, accepted and clamped future-larger tables, and sanitized
failed outputs. Invalid module, output, integer type, function, availability, dominance, and
insertion-point shapes were rejected before mutation. An exact Slice 14 provider retained bitwise
OR and gated XOR after discovery but before module creation or libNVVM use.

Direct NVVM and NVRTC exposed matching `[64, 32, 32]` parameter widths and each showed token-safe
`xor.b32` plus the expected global u32 store. CUDA 12.9 `ptxas` accepted both outputs. On the RTX
5090, both runtime lanes produced `0x66` for `0x5a ^ 0x3c`, `-305419897` for
`-1 ^ 0x12345678`, `15` for `-16 ^ -1`, and `-1` for `0 ^ -1`. The first complete focused NVVM
run passed 108/108 with no correction or rerun required. The established preservation runs also
passed 1/1, 2/2, 1/1, 3/3, 2/2, and 1/1.

Later on 2026-08-27, Slice 16 built the Release LLVM 14.0.6 provider and Debug host. The verified
provider fixture emitted exactly one `xor i32` with an all-ones (`-1`) operand whose result feeds
the established address-space-1 store, and no `xor i64`. The fake direct-route graph proved that
scalar parameter 1 is the exact BitNot operand, that the BitNot result is the stored value, and that
unrelated callbacks remain unused. Negotiation retained the exact 296-byte Slice 15 prefix for
bitwise-XOR programs, rejected every partial size from 297 through 303 bytes and a null complete
operation, accepted and clamped future-larger tables, and sanitized failed outputs. Invalid unary
module, output, integer type, function, availability, dominance, and insertion-point shapes were
rejected before mutation, while every predecessor binary validation test remained green. An exact
Slice 15 provider retained bitwise XOR and gated BitNot after discovery but before module creation
or libNVVM use.

Direct NVVM and NVRTC exposed matching `[64, 32]` parameter widths and each showed exact
token-boundary `not.b32` plus the expected global u32 store. NVRTC included its normal address
conversion while the direct route used the raw pointer; the semantic store contract remained the
same. CUDA 12.9 `ptxas` accepted both outputs. On the RTX 5090, both runtime lanes produced `-1`
for `~0`, `0` for `~-1`, `-1431655766` for `~0x55555555`, and `15` for `~-16`. The first complete
focused NVVM run passed 116/116. Preservation passed 1/1 parser, 2/2 routing/hash, 1/1 unsupported
boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

Later on 2026-08-27, Slice 17 built the Release LLVM 14.0.6 provider and Debug host. The verified
provider fixture emitted exactly one unflagged `sub i32 0, %x` whose result feeds the established
address-space-1 store, and no `i64`, `nsw`, or `nuw` variant. The fake direct-route graph proved
that scalar parameter 1 is the exact Negate operand, that the Negate result is the stored value,
and that unrelated callbacks, including BitNot, remain unused. Negotiation retained the exact
304-byte Slice 16 prefix for BitNot programs, rejected every partial size from 305 through 311
bytes and a null complete operation, accepted and clamped future-larger tables, reported the
capability in identity, and sanitized failed outputs. Invalid unary shapes were rejected before
mutation while the BitNot predecessor tests remained green. An exact Slice 16 provider compiled
BitNot, then gated Negate after one discovery but before module creation or libNVVM use.

Direct NVVM and NVRTC exposed matching `[64, 32]` parameter widths, exact token-boundary
`neg.s32`, and the expected global u32 store. NVRTC included its normal address conversion while
the direct route used the raw pointer; neither route used `sub.s32` or `not.b32`. CUDA 12.9
`ptxas` accepted both outputs. On the RTX 5090, both runtime lanes produced `0`, `-1`, `7`, and
`-2147483648` for inputs `0`, `1`, `-7`, and `INT_MIN`, respectively. The first complete focused
NVVM run passed 124/124. Preservation passed 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary,
3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

The Slice 18 baseline on 2026-08-27 measured the selected CUDA 12.9
`nvvm/libdevice/libdevice.10.bc` as 486,144 bytes, UTC `2025-05-27 09:50:51`, with SHA-256
`CD2824F8DD3F862B6B9259086F49F6CB56CA2547E14C61DE889C1C0D4A7DB175`. Explicit NVRTC emitted f32
parameter/global-store PTX for scalar multiply-add and inlined its sine implementation. The direct
f32 arithmetic probe stopped while validating an entry-point parameter; the `sin(float)` probe
stopped while collecting its float-returning target-helper call closure. Both failures occurred
before provider discovery. These measurements establish the negative f32/helper boundary and
reference route only; they do not test intrinsic-helper matching.

The complete focused Slice 18 NVVM suite passed 132/132. The compiler-level libdevice sine test
found the named entry and a global store with no unresolved `.extern .func`; `ptxas` from the same
CUDA 12.9 root accepted the PTX for `sm_75`. On the RTX 5090, inputs `0`, `0.5`, `-1.25`, and `20`
matched host `sinf` within `2e-6`. Preservation passed 1/1 parser, 2/2 routing/hash, 1/1 unsupported
boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

The Slice 19 baseline on 2026-08-27 measured final linked Slang IR containing exact
`atomicAdd(destination, delta, 0)` for the established read-write device `Ptr<int>` destination;
the literal zero is Relaxed and the signed-i32 result is the original stored value. CUDA 12.9 NVRTC
lowered the corresponding unsuffixed CUDA atomic to `atom.global.add.u32`, whose omitted semantic
and scope qualifiers represent the required relaxed/device behavior. The pre-change direct route
stopped at E52017 `atomicAdd`.

The LLVM 14 provider produces verifier-valid `atomicrmw add`, but CUDA 12.9 libNVVM's LLVM 7 reader
rejects its current bitcode record. Direct probes established the exact textual compatibility
surface: stable named parameters plus natural-alignment `atomicrmw` text verify and compile, while
explicit numeric parameter declarations and LLVM 14's `, align 4` suffix fail. The negotiated
NVVM-2.0 writer applies only those two conversions, and its semantic atomic count must match its
rewritten-line count. Direct PTX contains token-safe `atom.global.add.u32`; matching-root `ptxas`
and RTX 5090 runtime lanes pass for both discarded and consumed old-value results.

The isolated Slice 20 experiment then proved that exact upstream LLVM 7.0.1 bitcode carries the
complete Slice 19 graph through the same CUDA 12.9 libNVVM, `ptxas`, and runtime boundaries. That
technical result is preserved on `experiment/nvvm-llvm7-bitcode`; this branch retains the LLVM 14
text baseline until the dependency policy is decided.

The Slice 21 baseline measured final linked Slang IR containing exact
`cmpEQ(left, right) : Bool`, consumed by the established conditional/constant/phi/store graph. The
pre-change direct route stopped at E52017 `cmpEQ`; NVRTC accepted the source and emitted 32-bit
equality selection. The provider's shared binary comparison validator emits one verifier-valid
`icmp eq i32`, and the complete host route maps its `i1` result only to the already-supported
Boolean branch consumer.

Direct NVVM and NVRTC expose matching `[64, 32, 32]` parameters, token-safe 32-bit equality, and a
global 32-bit store. CUDA 12.9 `ptxas` accepts both outputs. RTX 5090 runtime cases cover equal zero,
equal negative, unequal signs, and unequal integer extremes on both routes. The focused suite
passes 148/148 and the preservation matrix passes 10/10.

The Slice 22 baseline measured final linked Slang IR containing exact
`cmpNE(left, right) : Bool`, consumed by the same established conditional/constant/phi/store
graph. The pre-change direct route stopped at E52017 `cmpNE`; NVRTC accepted the source and emitted
32-bit inequality selection. The provider's shared binary comparison validator emits one
verifier-valid `icmp ne i32`, and the complete host route maps its `i1` result only to the
already-supported Boolean branch consumer.

Direct NVVM and NVRTC expose matching `[64, 32, 32]` parameters, token-safe 32-bit equality
predicates, and a global 32-bit store. CUDA 12.9 `ptxas` accepts both outputs. RTX 5090 runtime
cases cover equal zero, equal negative, unequal signs, and unequal integer extremes on both routes.
The focused suite passes 156/156 and the preservation matrix passes 10/10.

The Slice 23 baseline measured final linked Slang IR containing exact
`cmpGT(left, right) : Bool`, consumed by the established conditional/constant/phi/store graph.
The pre-change direct route stopped at E52017 `cmpGT`; NVRTC accepted the source and emitted a
signed 32-bit ordered comparison. The provider's shared binary comparison validator emits one
verifier-valid `icmp sgt i32`, and the complete host route maps its `i1` result only to the
already-supported Boolean branch consumer.

Direct NVVM and NVRTC expose matching `[64, 32, 32]` parameters, signed 32-bit ordered predicates,
and a global 32-bit store. CUDA 12.9 `ptxas` accepts both outputs. RTX 5090 runtime cases cover
equal zero, equal negative, unequal signs in both directions, and both orderings of the integer
extremes on both routes. The focused suite passes 164/164 and the preservation matrix passes
10/10.

The Slice 24 baseline measured final linked Slang IR containing exact
`cmpLE(left, right) : Bool`, consumed by the established conditional/constant/phi/store graph.
The pre-change direct route stopped at E52017 `cmpLE`; NVRTC accepted the source and emitted a
signed 32-bit less-equal comparison. The provider's shared binary comparison validator emits one
verifier-valid `icmp sle i32`, and the complete host route maps its `i1` result only to the
already-supported Boolean branch consumer.

Direct NVVM and NVRTC expose matching `[64, 32, 32]` parameters, signed 32-bit ordered predicates,
and a global 32-bit store. CUDA 12.9 `ptxas` accepts both outputs. RTX 5090 runtime cases cover
equal zero, equal negative, unequal signs in both directions, and both orderings of the integer
extremes on both routes. The focused suite passes 172/172 and the preservation matrix passes
10/10.

The Slice 25 baseline measured final linked Slang IR containing exact
`cmpGE(left, right) : Bool`, consumed by the established conditional/constant/phi/store graph.
The pre-change direct route stopped at E52017 `cmpGE`; NVRTC accepted the source and emitted a
signed 32-bit greater-equal comparison. The provider's shared binary comparison validator emits one
verifier-valid `icmp sge i32`, and the complete host route maps its `i1` result only to the
already-supported Boolean branch consumer.

Direct NVVM and NVRTC expose matching `[64, 32, 32]` parameters, signed 32-bit ordered predicates,
and a global 32-bit store. CUDA 12.9 `ptxas` accepts both outputs. RTX 5090 runtime cases cover
equal zero, equal negative, unequal signs in both directions, and both orderings of the integer
extremes on both routes. The focused suite passes 180/180 and the preservation matrix passes
10/10.

The Slice 26 baseline measured one exact raw
`HLSLRWStructuredBufferType(Int, DefaultBufferLayout)` entry parameter and
`rwstructuredBufferGetElementPtr(destination, index)` producer. The pre-change direct route
stopped at E52017 on the resource entry parameter. The conventional global spelling instead
retained `ConstantBuffer<GlobalParams>`, `get_field_addr`, and a load, confirming that it is a
separate ABI rather than an alternative raw-resource spelling.

The provider maps the raw resource to exact `{ i32 addrspace(1)*, i64 }`, extracts field zero, and
applies one non-`inbounds` signed-i32 GEP before the established store. Direct NVVM and NVRTC expose
the same aligned 16-byte aggregate plus i32 index parameters, first-u64 data-pointer load, signed
index scaling, and global u32 store. CUDA 12.9 `ptxas` accepts both, and RTX 5090 execution stores
42 through the exact `{device pointer, count}` launch value. The focused suite passes 188/188 and
the preservation matrix passes 10/10.

## Settled and Open Decisions

Settled decisions are the support contract at the top of this document, the parallel backend
shape, the continued NVRTC default, the binary artifact shape, the separate exact LLVM 14 builder
and isolated LLVM 7 feasibility branch,
the frozen-V1/append-only-V2 diagnostic ABI, the coherent AS1 scalar-memory capability and
differential contract, the Slice 5 target-option/routing boundary, and the Slice 6 exact
empty-compute subset. Slice 7 settles the raw `[CUDAKernel]` signed-`i32`/device-pointer parameter
ABI, the append-only V2 scalar-control-flow prefix, and dominance and ownership preflight at both
the Slang IR and provider boundaries. The direct route uses one session-owned builder for hashing
and code generation, requires the V2 verifier boundary, and enters the established downstream
continuation as an internal LLVM-IR kernel artifact. Providers through Slice 17 supply bitcode;
Slice 19 providers may advertise the audited NVVM-2.0 assembly wire dialect explicitly.

Slice 8 settles exactly representable signed-i32 executable constants, signed-i32 block parameters
and branch arguments lowered as LLVM phis, canonical `IRLoop` target edges, and signed-i32
loop-carried state. It also settles the append-only V2 scalar-SSA prefix and its complete-CFG,
predecessor-edge dominance boundary. These claims cover the demonstrated merge and finite sum loop;
they do not imply general termination analysis or other value types.

Slice 9 settles the finite direct-call closure rooted at one selected raw CUDA kernel, canonical
module-owned helper identities, signed-i32 helper parameters/results, valued returns, and the
append-only scalar-function provider prefix. It also settles declaration-before-body emission and
the rule that insertion-block ownership, rather than a second ambient function cursor, identifies
the caller. These claims do not include external or indirect calls, recursion, void/pointer helpers,
or preservation of source function attributes.

Slice 10 settles canonical `IRGetOffsetPtr` lowering for signed-i32 element offsets on the existing
device-`int` pointer ABI, exact equality between result and base pointer types including access,
ordinary non-`inbounds` LLVM GEP construction, and the append-only scalar-pointer-arithmetic
provider prefix. It also settles complete pre-mutation ownership, availability, typed-pointer, and
sized-pointee validation at the provider boundary. These claims do not include `IRGetElementPtr`,
unsigned or wider offsets, other pointees or address spaces, arrays, aggregates, globals, or shared
memory.

Slice 11 settles canonical two-operand `IRGetElementPtr` lowering for nonempty fixed signed-i32
arrays behind read or read-write device entry-point pointers. It settles the canonical relation
between an array-pointer base and its scalar-pointer result: the address space and access remain
equal, the result pointee is the array element, and the CUDA natural-layout producer may give the
base and result different layout spellings. It also settles the coherent append-only
`getArrayType`/`emitArrayElementPointer` provider prefix, the frozen exact Slice 10 compatibility
boundary, and ordinary non-`inbounds` `{i32 0, index}` LLVM GEP construction after complete
pre-mutation validation. These claims do not include other element types or array shapes, array
values, other aggregates, local or global storage, helper array pointers, unsigned or wider
indices, additional address spaces, or shared memory.

Slice 12 settles canonical two-operand `kIROp_Mul` lowering for signed-i32 values already available
through the established scalar program structure. It settles the dedicated append-only
`emitIntegerMultiply` provider operation, exact Slice 11 compatibility, a 272-byte complete x64
prefix, pre-mutation equal-integer-type and ownership/dominance validation, and the rule that
Slang's preflight owns signed-i32 policy while LLVM's signless integer operation owns instruction
construction. These claims do not include other integer widths or signedness, floating-point or
aggregate multiplication, overflow variants, fused operations, division, remainder, shifts,
bitwise operations, or casts.

Slice 13 settles canonical two-operand `kIROp_BitAnd` lowering for signed-i32 values already
available through the established scalar program structure. It settles the dedicated append-only
`emitIntegerBitAnd` provider operation, exact 272-byte Slice 12 compatibility, a 280-byte complete
x64 prefix, pre-mutation equal-integer-type and ownership/dominance validation, and the division of
responsibility in which Slang owns signed-i32 policy while LLVM owns signless `CreateAnd`
construction. These claims do not include `kIROp_ConstexprBitAnd`, bitwise OR/XOR/NOT, shifts,
logical operations, other integer widths or signedness, vectors, matrices, aggregates, or new ABI
and storage shapes.

Slice 14 settles canonical two-operand `kIROp_BitOr` lowering for signed-i32 values already
available through the established scalar program structure. It settles the dedicated append-only
`emitIntegerBitOr` provider operation, exact 280-byte Slice 13 compatibility, a 288-byte complete
x64 prefix, pre-mutation equal-integer-type and ownership/dominance validation, and the division of
responsibility in which Slang owns signed-i32 policy while LLVM owns signless `CreateOr`
construction. These claims do not include `kIROp_ConstexprBitOr`, logical `kIROp_Or`, bitwise
XOR/NOT, shifts, other integer widths or signedness, vectors, matrices, aggregates, or new ABI and
storage shapes.

Slice 15 settles canonical two-operand `kIROp_BitXor` lowering for signed-i32 values already
available through the established scalar program structure. It settles the dedicated append-only
`emitIntegerBitXor` provider operation, exact 288-byte Slice 14 compatibility, a 296-byte complete
x64 prefix, pre-mutation equal-integer-type and ownership/dominance validation, and the division of
responsibility in which Slang owns signed-i32 policy while LLVM owns signless `CreateXor`
construction. These claims do not include `kIROp_ConstexprBitXor`, bitwise NOT, shifts, division,
remainder, other integer widths or signedness, vectors, matrices, aggregates, or new ABI and
storage shapes.

Slice 16 settles canonical one-operand `kIROp_BitNot` lowering for signed-i32 values already
available through the established scalar program structure. It settles the dedicated append-only
`emitIntegerBitNot` provider operation, exact 296-byte Slice 15 compatibility, a 304-byte complete
x64 prefix, and a shared unary integer validator for scalar type, ownership, availability, and
dominance. The established binary validator composes two such unary checks plus exact type equality,
so its behavior remains unchanged. Slang owns signed-i32 policy while LLVM owns signless
`CreateNot` construction as an all-ones `xor i32`. These claims do not include
`kIROp_ConstexprBitNot`, logical `kIROp_Not`, arithmetic negation, shifts, division, remainder,
other integer widths or signedness, vectors, matrices, aggregates, or new ABI and storage shapes.

Slice 17 defines canonical one-operand `kIROp_Neg` lowering for signed-i32 values already available
through the established scalar program structure. It appends the dedicated `emitIntegerNegate`
provider operation after the exact 304-byte Slice 16 prefix, yielding a 312-byte complete x64
prefix, and reuses the shared unary integer validator for type, ownership, availability, and
dominance. Slang owns the signed-i32 and wrapping policy; LLVM owns plain unflagged `CreateNeg`
construction as `sub i32 0, value`. These claims do not include `kIROp_ConstexprNeg`, unsigned,
narrow, wide, floating-point, vector, matrix, or aggregate negation, shifts, division, remainder,
or new ABI and storage shapes.

Slice 18 defines downstream CUDA-device-library demand and fp32 compile policy without widening the
direct emitter or its builder ABI. The terminal naturally aligned, pointer-sized
`requiresCUDADeviceLibrary = 0` field preserves older compatible option prefixes and makes library
loading explicit; zero means false and any nonzero value means true. A true request uses only the
toolkit root retained from the selected libNVVM library, reads exact
`nvvm/libdevice/libdevice.10.bc` bytes before program creation, and adds user then library modules
through the lazy API or the eager compatibility path when that optional symbol is absent. The typed
fp32 mode and denormal fields own the four managed option families; unsupported fp16/fp64 denormal
policy and compiler-specific overrides are rejected before mutation. Direct Slang float ABI,
arithmetic, helpers, and intrinsic recognition remain outside this claim; the f32 arithmetic case
stops at its entry-point parameter, while sine stops at its float-returning helper result.

Slice 19 settles exact canonical Relaxed signed-i32 atomic add through an already-supported
canonical read-write device-i32 pointer. Its terminal `emitRelaxedGlobalI32AtomicAdd` operation
fixes AS1, typed `i32`, four-byte alignment, LLVM `monotonic` ordering, and default System sync
scope rather than exporting policy knobs across the private ABI. The operation and negotiated
NVVM-2.0 assembly writer form one complete 328-byte x64 prefix after the frozen 312-byte Slice 17
prefix. Older providers continue to receive bitcode; full providers pass the direct-emission,
negative, PTX, `ptxas`, and runtime evidence above.

Slice 21 settles exact two-operand `kIROp_Eql` lowering for signed-i32 values already available
through the established scalar program structure. It appends the dedicated `emitIntegerEqual`
provider operation after the exact 328-byte Slice 19 prefix, yielding a 336-byte complete x64
prefix, and reuses the shared binary integer validator for type, ownership, availability, and
dominance before exact `ICMP_EQ` construction. Slang owns the signed-i32 operand and canonical
Boolean-result policy; LLVM owns the signless integer comparison and `i1` result. The Boolean may
feed established control flow, but this slice adds no Boolean ABI, storage, return, or phi surface.
These claims do not include inequality, other ordered comparisons, unsigned/wide/floating-point/
pointer equality, vectors, matrices, aggregates, resources, or new ABI and storage shapes.

Slice 22 settles exact two-operand `kIROp_Neq` lowering for signed-i32 values already available
through the established scalar program structure. It appends the dedicated `emitIntegerNotEqual`
provider operation after the exact 336-byte Slice 21 prefix, yielding a 344-byte complete x64
prefix, and reuses the shared binary integer validator for type, ownership, availability, and
dominance before exact `ICMP_NE` construction. Slang owns the signed-i32 operand and canonical
Boolean-result policy; LLVM owns the signless integer comparison and `i1` result. The Boolean may
feed established control flow, but this slice adds no Boolean ABI, storage, return, or phi surface.
These claims do not include ordered comparisons, unsigned/wide/floating-point/pointer inequality,
vectors, matrices, aggregates, resources, or new ABI and storage shapes.

Slice 23 settles exact two-operand `kIROp_Greater` lowering for signed-i32 values already available
through the established scalar program structure. It appends the dedicated
`emitIntegerSignedGreaterThan` provider operation after the exact 344-byte Slice 22 prefix,
yielding a 352-byte complete x64 prefix, and reuses the shared binary integer validator for type,
ownership, availability, and dominance before exact `ICMP_SGT` construction. Slang owns the
signed-i32 operand and canonical Boolean-result policy; LLVM owns the signless integer comparison
and `i1` result. The measured final producer remains `kIROp_Greater`, so this consumer neither
reverses operands nor accepts an alternative comparison spelling. These claims do not include
less-equal, greater-equal, unsigned/wide/floating-point/pointer ordered comparisons, vectors,
matrices, aggregates, resources, or new ABI and storage shapes.

Slice 24 settles exact two-operand `kIROp_Leq` lowering for signed-i32 values already available
through the established scalar program structure. It appends the dedicated
`emitIntegerSignedLessEqual` provider operation after the exact 352-byte Slice 23 prefix, yielding
a 360-byte complete x64 prefix, and reuses the shared binary integer validator for type, ownership,
availability, and dominance before exact `ICMP_SLE` construction. Slang owns the signed-i32
operand and canonical Boolean-result policy; LLVM owns the signless integer comparison and `i1`
result. The measured final producer remains `kIROp_Leq`, so this consumer does not reconstruct it
from other comparisons or accept an alternative spelling. These claims do not include
greater-equal, unsigned/wide/floating-point/pointer ordered comparisons, vectors, matrices,
aggregates, resources, or new ABI and storage shapes.

Slice 25 settles exact two-operand `kIROp_Geq` lowering for signed-i32 values already available
through the established scalar program structure. It appends the dedicated
`emitIntegerSignedGreaterEqual` provider operation after the exact 360-byte Slice 24 prefix,
yielding a 368-byte complete x64 prefix, and reuses the shared binary integer validator for type,
ownership, availability, and dominance before exact `ICMP_SGE` construction. Slang owns the
signed-i32 operand and canonical Boolean-result policy; LLVM owns the signless integer comparison
and `i1` result. The measured final producer remains `kIROp_Geq`, so this consumer does not
reconstruct it from other comparisons or accept an alternative spelling. These claims do not
include unsigned/wide/floating-point/pointer ordered comparisons, vectors, matrices, aggregates,
resources, or new ABI and storage shapes.

Slice 26 begins the resource-ABI bucket with exact raw `[CUDAKernel]`
`RWStructuredBuffer<int, DefaultLayout>` parameters and their canonical
`kIROp_RWStructuredBufferGetElementPtr` producer. It appends one coherent type/addressing provider
capability after the exact 368-byte Slice 25 x64 prefix, yielding a 384-byte complete x64 prefix.
The provider's single structural source of truth is `{ i32 addrspace(1)*, i64 }`; it validates the
exact aggregate, signed-i32 index, ownership, availability, dominance, and insertion state before
one field-zero extraction and non-`inbounds` GEP. Conventional global parameter blocks are a
different measured ABI and remain unsupported, as do read-only buffers and non-i32 resource
elements. The direct consumer does not flatten the launch value, reconstruct source syntax, or
accept alternative structural spellings.

The following remain open until their named slice supplies evidence:

- packaging and update policy for the optional NVVM builder module, including whether production
  owns LLVM 7.0.1 plus an older CMake frontend or LLVM 14.0.6 plus negotiated text;
- the CUDA toolkit and GPU CI matrix;
- whether NVVM IR should become a public compile target;
- conventional shader-entry parameters and raw CUDA parameters beyond signed `i32`, device
  pointers, fixed i32 array pointers, and exact raw `RWStructuredBuffer<int>`;
- external/indirect calls, richer helper ABI, and scalar operations beyond the established
  signed-i32 add, subtract, multiply, bitwise-AND, bitwise-OR, bitwise-XOR, bitwise-NOT,
  arithmetic-negate and comparison family, plus scalar float32 add, subtract, multiply, divide,
  negate, ordered equality, unordered inequality, ordered greater-than, ordered less-than,
  ordered less-than-or-equal, and ordered greater-than-or-equal;
- pointer and aggregate addressing beyond signed-i32 scalar offsets and the exact fixed-i32 device
  array subset, including other `IRGetElementPtr` shapes, array values, structs, globals, shared
  memory, and additional address spaces;
- every other atomic operation, memory order, value type, pointer shape, and address space, plus a
  production decision between the proven isolated LLVM 7 bitcode writer, the experimental text
  bridge, and a future purpose-built bitcode writer;
- wave/subgroup operations, including their lane, mask, convergence, and intrinsic contracts;
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
