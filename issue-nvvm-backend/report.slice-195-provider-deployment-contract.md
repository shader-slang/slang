# Slice 195: Enforce the provider deployment contract

## Motivation

The production policy said the compiler-matched LLVM 14 provider should ship beside Slang, but the
unset-environment implementation requested only the logical name `slang-llvm-nvvm`. That delegates
selection to platform-wide loader order and does not reliably search an executable's directory on
Linux or macOS. Meanwhile, the standalone provider CMake project had no install rule to create the
documented layout:

```text
<prefix>/bin/slang-test[.exe]
<prefix>/bin/slang-llvm-nvvm.<platform suffix>
```

Real-provider unit helpers passed the executable directory explicitly, so they did not prove this
production session behavior.

## Proposed solution

Resolve one authoritative provider path per global session. A nonempty
`SLANG_NVVM_BUILDER_PATH` wins; otherwise use `PlatformUtil::getInstancePath`, the repository's
running-executable-directory contract. Cache that path together with the first success or failure,
name it in E52016, and never continue to PATH or NVRTC.

Give the independent provider project an optional `slang-llvm-nvvm` install component that puts
every platform's MODULE artifact in the install bindir. Document the same-checkout, exact-ABI,
atomic-update policy next to the provider source.

## Change summary

- Replaced the session's "explicit path" state with the actual resolved search path.
- Made executable adjacency the only no-override production location and preserved the environment
  variable as an authoritative directory-or-file override.
- Strengthened fake-boundary tests for the resolved location, one-attempt success/failure caching,
  diagnostic location, and absence of NVRTC fallback.
- Added the standalone provider CMake install component and operator-facing build/install/update
  documentation.
- Added fresh frozen/discovery snapshots and representative workload measurements.
- Kept provider ABI revision 35 and all compiler/provider IR behavior unchanged.

## Concepts and vocabulary

**Adjacent deployment** places the provider in the same `bin` directory as the process loading
Slang. **Resolved search path** is the single directory or file a session chose before it attempted
to load the provider. **Compiler-matched** means the provider is built from the same checkout and
must expose the exact ABI revision expected by that compiler. **Atomic update** replaces compiler
and provider package contents together rather than mixing revisions.

## Process report

`Session::getOrLoadNVVMIRBuilder` is the canonical owner of provider discovery because both shader
hashing and `CodeGenContext::emitNVVMForEntryPoints` use its retained `NVVMIRBuilder`. It now reads
the environment override once. If absent, it asks `PlatformUtil::getInstancePath` for the running
executable directory and passes that nonempty path to `NVVMIRBuilder::load`.
`DownstreamCompilerUtil::loadSharedLibrary` decorates the module name inside that exact directory.

This is a deployment-state branch, not a shader-IR shape special case. The directory comes from the
host platform before any provider call, and all compute programs share it. If the platform cannot
resolve an executable directory, the session caches that failure; it does not use the old logical
PATH search. If a configured or adjacent module is missing, stale, or ABI-incompatible, the same
failure and path are returned for every request on that session. Replacing the session's shared
library loader still clears the cached provider, as before.

`CodeGenContext::emitNVVMForEntryPoints` now diagnoses the returned search path, including the
default adjacent directory. The fake success test gets an entry-point hash and then emits code,
observing one load request. The fake missing-provider test compiles twice and observes one failed
request, two E52016 diagnostics naming the selected directory, zero builder module creations, and
zero libNVVM program creations. Because the route is explicitly direct NVVM, no NVRTC compiler is
queried or used.

The provider project's `install(TARGETS)` rule sends both Windows runtime modules and Unix/macOS
library modules to `${CMAKE_INSTALL_BINDIR}` under component `slang-llvm-nvvm`. A clean Slice 195
prefix contained exactly `bin/slang-llvm-nvvm.dll`, 2,750,464 bytes, with SHA-256
`46D4EEC7609D9B5619532A2476DB51BD11CE301636E4CDBC2B6F67FBEE0A6CF8`. Its hash matched the freshly
built provider. Serialization, direct empty-kernel compilation, libNVVM compilation, and SM70
`ptxas` assembly passed when `SLANG_NVVM_BUILDER_PATH` named that installed directory.

The first no-override integration run found an older 2,693,632-byte provider already beside
`slang-test.exe` and exact ABI negotiation rejected it. After copying the freshly built module and
compiler together, the same no-override tests passed. This is evidence for keeping exact revision
matching and package-atomic updates; accepting the stale provider would have hidden an incomplete
deployment. The installed README therefore says to build from the same checkout, replace both
artifacts together, and create a new global session after an on-disk update.

The special-case inventory has three entries. Environment precedence survives because it is the
explicit deployment/development input and has deterministic failure semantics. Executable-directory
selection survives because it is the package layout established by the new install component and
is common to every workload. Failure caching survives because provider identity affects the shader
hash and code generation must use the same retained outcome. The draft logical-name fallback was
removed during self-review because it contradicted deterministic adjacency. There is no new IR
helper, syntax reconstruction, compatibility fallback, provider operation, or ABI branch.

Release compiler/test and isolated LLVM 14 provider builds passed. The selected NVVM unit prefix
passes 439/439, including the strengthened discovery tests, and the permanent NVVM category passes
102/102. Both installed-directory and no-override adjacent real-provider paths pass direct
compilation and PTX assembly.

Frozen corpus v1 retains exactly 452 identities/448 sources and 427 healthy references. O0, O3,
and both remain 423/423/423 with zero classification change and zero old-correct regression. Its
all-row totals remain 449 native correct plus three infrastructure; each direct mode remains 437
correct, 14 preflight, and one infrastructure. Discovery separately retains exactly 82 identities
and 72 healthy references at 72/72/72, also with zero classification change. Its direct all-row
totals remain 72 correct, seven infrastructure, one runtime mismatch, and two preflight per mode.
No workload is newly unlocked because discovery policy, not code generation, changed.

All three representative gates still assemble at direct O3 for SM70, SM80, and SM90. Their direct
O3 PTX sizes remain 919, 793, and 1,404 bytes. At SM70, three-run median compile times were
256.7/233.2/236.6 ms through direct O3 versus 373.4/345.6/356.7 ms through native NVRTC O3. These
remain exploratory startup-inclusive measurements, not controlled kernel-runtime benchmarks.
