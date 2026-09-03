# Enforce the compiler-matched provider deployment contract

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The maintainers have
explicitly asked that each direct-NVVM slice commit include its plan, so this plan is a deliberate
exception to the repository's usual working-log policy.

## Purpose and Observable Result

Make the optional LLVM 14 provider deployable and discoverable by one deterministic rule: when
`SLANG_NVVM_BUILDER_PATH` is unset, a session loads `slang-llvm-nvvm` from the running executable's
directory; when it is set, that path is authoritative. Cache one success or failure per global
session, report the actual searched location, and provide a CMake install component that places the
provider in `bin` beside Slang executables.

## Progress

- [x] (2026-09-03) Audited runtime discovery, existing cache/failure tests, standalone provider
  CMake, and the documented production policy.
- [x] (2026-09-03) Implemented the deterministic resolved-search-path contract and updated
  fake-boundary tests.
- [x] (2026-09-03) Added and exercised the optional adjacent-provider install component.
- [x] (2026-09-03) Ran focused and broad build/test/deployment gates, then confirmed both corpus
  scores are unchanged.
- [x] (2026-09-03) Completed self-review, durable documentation, the five-part report, and prepared
  the Slice 195 commit.

## Surprises and Discoveries

- The session already caches both provider success and failure and invalidates that cache when its
  shared-library loader changes. Existing tests prove the one-attempt behavior.
- With no environment override, `NVVMIRBuilder::load` currently passes only the logical name to the
  platform loader. That can search process/global loader paths and does not enforce adjacency.
- Real-provider unit helpers explicitly pass `context->executableDirectory`, so their success does
  not prove the production session's unset-environment behavior.
- The independent `source/slang-llvm-nvvm` project has export isolation but no `install()` rule.
- The first no-override real-provider run found a stale provider already beside `slang-test.exe`.
  Exact ABI negotiation rejected it; replacing it with the freshly built module made the same test
  pass. This directly demonstrates why compiler/provider package updates must be atomic.
- Self-review found that the first implementation draft retained logical loader lookup if the
  platform could not resolve an executable directory. That contradicted deterministic adjacency,
  so the fallback was removed before final validation.

## Decision Log

- Decision: resolve the default provider directory with `PlatformUtil::getInstancePath`, which is
  already the repository contract for the running executable directory. Do not probe PATH or a
  second location after failure. Date/author: 2026-09-03, Codex.
- Decision: retain `SLANG_NVVM_BUILDER_PATH` as an authoritative directory-or-file override and
  cache the resolved path together with the load result. Date/author: 2026-09-03, Codex.
- Decision: install all platform forms of the MODULE target into `${CMAKE_INSTALL_BINDIR}` and use
  a dedicated optional component. Runtime adjacency is more important than conventional Unix
  library placement for this isolated compiler plugin. Date/author: 2026-09-03, Codex.
- Decision: keep ABI revision 35. Discovery and packaging change where the same exact provider is
  found; they add no provider operation or binary interface. Date/author: 2026-09-03, Codex.

## Outcomes and Retrospective

The standalone provider now installs as one optional component into `bin`, and both a clean
installed-directory override and default executable-adjacent discovery pass real serialization,
direct libNVVM compilation, and PTX assembly. Success and failure are one-attempt session-cached,
the exact searched location appears in E52016, and no implicit PATH or NVRTC fallback remains.

Release compiler/test and isolated-provider builds pass. The selected prefix is 439/439 and the
permanent category is 102/102. Frozen v1 remains exactly 452/427 at 423/423/423 and discovery
remains exactly 82/72 at 72/72/72, each with zero classification change. All three representative
workloads retain their PTX sizes and assemble for SM70/SM80/SM90. ABI revision 35 is unchanged.

## Context and Current Pipeline

`Session::getOrLoadNVVMIRBuilder` reads `SLANG_NVVM_BUILDER_PATH` once, calls
`NVVMIRBuilder::load`, and stores its result and builder. An empty path reaches
`DownstreamCompilerUtil::loadSharedLibrary`, which asks the loader for the logical name
`slang-llvm-nvvm`. Direct code generation calls the same session method after canonical preflight;
entry-point hashing calls it earlier and incorporates the loaded provider identity.

The intended package places an isolated, statically linked LLVM 14 MODULE provider beside Slang
binaries. Explicit direct selection must never fall back to NVRTC. The new rule makes that intended
filesystem relationship the default discovery source rather than relying on platform search order.

## Scope and Non-Goals

In scope are provider search-path resolution, cached location/result behavior, exact diagnostics,
the standalone provider install target, unit tests, and installation/runtime validation. Out of
scope are libNVVM discovery, CUDA toolkit selection, public package composition, download/update
services, ABI changes, provider operations, compiler IR, corpus selection, and compatibility with
the experimental generic-library-search behavior.

## Architecture and Invariants

- A session resolves one provider location and makes one load attempt until its loader is replaced.
- A nonempty `SLANG_NVVM_BUILDER_PATH` is authoritative; failure never probes adjacency or PATH.
- Without an override, hosts must report the running executable directory. Failure to resolve that
  directory is itself cached and diagnosed; production discovery does not fall back to PATH.
- Explicit direct NVVM never silently invokes NVRTC.
- The diagnostic names the resolved directory or logical fallback actually searched.
- The provider package is compiler-matched and forward-only: source commit, ABI revision, and build
  recipe move together and installation replaces the module atomically as one artifact.

## Interfaces and Dependencies

Rename the private session cached path from "explicit" to "search" terminology; no public Slang
API changes. Use existing `PlatformUtil`, `Path`, `DownstreamCompilerUtil`, and exact ABI negotiation.
The provider CMake project will include `GNUInstallDirs` and install its MODULE target's runtime and
library artifacts into `${CMAKE_INSTALL_BINDIR}` under component `slang-llvm-nvvm`. Its local README
is the operator-facing source of truth for building, installing, discovering, and updating it.

## Milestones

1. Resolve and cache the authoritative search path in `Session::getOrLoadNVVMIRBuilder`; return it
   to diagnostics and update the private member name.
2. Make fake loaders accept and retain decorated provider paths; strengthen success/failure cache
   tests to prove one resolved location and no fallback.
3. Add the optional install component to `source/slang-llvm-nvvm/CMakeLists.txt`.
4. Configure/build/install the provider into a clean prefix below `build/`, verify the exact
   artifact layout, and load it through that installed directory.
5. Run focused/broad unit and runtime gates, confirm frozen/discovery evidence, complete the
   input-shape audit and documentation, and commit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox. Acceptance requires:

- Release compiler/unit-test and isolated provider builds pass.
- Focused fake tests prove resolved path, one-attempt success caching, one-attempt failure caching,
  reported location, and no NVRTC fallback.
- `cmake --install --component slang-llvm-nvvm` creates exactly the platform provider artifact
  below a temporary `bin`, and focused real-provider/direct runtime tests pass through that path.
- The selected NVVM unit prefix and permanent direct-NVVM category pass.
- Frozen v1 remains 423/423/423 over 427 with no old-correct regressions; discovery remains
  72/72/72 over 72.
- `git diff --check` and formatting pass.

## Failure and Recovery

If the strict adjacent default breaks an intentional deployment, the supported recovery is to set
`SLANG_NVVM_BUILDER_PATH` to its directory or exact module file, not to restore implicit PATH
probing. Install validation uses a fresh prefix below `build/` and is safely repeatable. Compiler and
provider code remain independently buildable throughout.

## Artifacts and Hand-Off

Commit the completed plan with compiler/CMake/test changes, the five-part report, and durable design
and capability-ledger updates. Keep configured provider builds, install prefixes, and logs under
ignored `build/`.
