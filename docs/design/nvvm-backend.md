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
16. libdevice and floating-point policy;
17. atomics and wave operations;
18. resources and optimization-quality work; and
19. advanced capabilities and production-readiness evaluation.

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
signed-i32 bitwise OR. Bitwise XOR remains the next bounded canonical scalar operation for Slice
15; NOT, shifts, and the other richer scalar policies remain separate decisions.

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

## Settled and Open Decisions

Settled decisions are the support contract at the top of this document, the parallel backend
shape, the continued NVRTC default, the binary artifact shape, the separate pinned LLVM 14 builder,
the frozen-V1/append-only-V2 diagnostic ABI, the coherent AS1 scalar-memory capability and
differential contract, the Slice 5 target-option/routing boundary, and the Slice 6 exact
empty-compute subset. Slice 7 settles the raw `[CUDAKernel]` signed-`i32`/device-pointer parameter
ABI, the append-only V2 scalar-control-flow prefix, and dominance and ownership preflight at both
the Slang IR and provider boundaries. The direct route uses one session-owned builder for hashing
and code generation, requires the V2 verifier boundary, and enters the established downstream
continuation as an internal LLVM-bitcode kernel artifact.

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

The following remain open until their named slice supplies evidence:

- packaging and update policy for the optional LLVM 14 NVVM builder module;
- the CUDA toolkit and GPU CI matrix;
- whether NVVM IR should become a public compile target;
- conventional shader-entry parameters and raw CUDA parameters beyond signed `i32` and device
  pointers;
- external/indirect calls, richer helper ABI, and scalar operations beyond the established
  signed-i32 add, subtract, multiply, bitwise-AND, bitwise-OR, and less-than subset;
- pointer and aggregate addressing beyond signed-i32 scalar offsets and the exact fixed-i32 device
  array subset, including other `IRGetElementPtr` shapes, array values, structs, globals, shared
  memory, and additional address spaces;
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
