# Prove a Typed-Pointer NVVM Bitcode Path

This ExecPlan follows `.agent/PLANS.md`. It is an active working log under `issue-nvvm-backend/` and
must remain out of commits. Keep it current so another session can resume from the recorded
commands, evidence, and decisions; distill durable conclusions into `docs/design/nvvm-backend.md`.

## Purpose and Observable Result

This slice tests whether the binary NVVM IR artifact boundary is viable for the agreed
pre-Blackwell path and selects the next writer implementation direction. It does not implement a
Slang-side writer or begin general Slang IR lowering. At completion, the repository must contain
evidence for one of two honest outcomes:

1. a named, reproducible external typed-pointer bitcode writer produces a minimal NVVM IR 2.0
   kernel that the installed CUDA 12.2 libNVVM verifies, compiles to PTX, and whose PTX `ptxas`
   assembles, establishing compatibility and the consumer artifact contract; or
2. the candidate writers fail, with their exact diagnostics recorded and the next architecture
   decision narrowed to a dedicated serializer or pinned LLVM component.

If the positive gate succeeds, `NVVMDownstreamCompiler` will accept an exact
`ObjectCode + LLVMIR + Kernel` artifact in addition to its bootstrap
`Assembly + LLVMIR + Kernel` input. An always-running fake test will prove byte-exact forwarding,
including embedded NULs, and an environment-aware real test will prove that the checked-in
bitcode fixture is accepted by libNVVM. Ordinary `-target ptx` compilation remains on NVRTC.

## Progress

- [x] (2026-08-25 21:55Z) Started the Slice 2 LLVM, artifact, test, and installed-toolkit audits.
- [x] (2026-08-25 21:55Z) Confirmed the local optional compiler is LLVM 21.1 and the CUDA 12.2 API
  contract names LLVM 7.0.1 bitcode; no `llvm-as` is installed on PATH.
- [x] (2026-08-25 22:12Z) Produced minimal kernel bitcode with LLVM 14.0.6, verified and
  compiled it through CUDA 12.2 libNVVM, and assembled the result with `ptxas`.
- [x] (2026-08-25 22:12Z) Audited the LLVM 21 boundary and used exploratory LLVM 16/NVRTC controls
  to separate dialect limitations from container-format limitations.
- [x] (2026-08-25 22:12Z) Selected a separate optional LLVM 14 typed-pointer NVVM builder module as
  the next implementation direction; the LLVM 21 CPU plugin remains independent.
- [x] (2026-08-25 22:12Z) Added the exact binary artifact contract and focused fake/real tests.
- [x] (2026-08-25 22:12Z) Built, ran `ptxas`, ran nearby regressions, and completed the
  principled-change audit.
- [x] (2026-08-25 22:12Z) Updated `docs/design/nvvm-backend.md` with the result and next-slice
  hand-off.
- [x] (2026-08-25 22:17Z) Incorporated independent ABI, fixture, and architecture reviews; added
  exact fixture-to-header regeneration tooling, narrowed claims to the evidence, recorded the
  multi-LLVM isolation invariant, and reran all focused validation.

## Surprises and Discoveries

- Observation: the official CUDA 12.2 libNVVM API requires LLVM 7.0.1 bitcode or text for
  `nvvmAddModuleToProgram`.
  Evidence: the installed `nvvm.h` and the CUDA 12.2 libNVVM API reference.

- Observation: NVIDIA's current `cuda-c-linking` sample documents LLVM 7 through 14 as suitable
  for its pre-Blackwell typed-pointer builder, while LLVM 15 defaults to opaque pointers.
  The sample submits text IR, so this range informs the writer choice but does not guarantee
  LLVM-14 bitcode compatibility. The checked-in fixture is the local bitcode evidence.
  Evidence: `Samples/7_libNVVM/README.md` in NVIDIA's `cuda-samples` repository and the fixture
  regeneration procedure.

- Observation: this checkout uses a prebuilt `slang-llvm.dll` reporting LLVM 21.1. Its public
  `LLVMBuilder` interface can produce textual assembly, host object code, and a JIT library, but
  exposes no bitcode serialization method. The prebuilt DLL exports only Slang entry points and no
  LLVM C bitcode API.
  Evidence: `slangc -llvm-version`, `ILLVMBuilder`, and the DLL export table.

- Observation: the current NVVM downstream data path already passes arbitrary blob bytes and a
  byte count to `nvvmAddModuleToProgram`; only its exact artifact descriptor gate is text-only.
  Evidence: `NVVMDownstreamCompiler::compile`.

- Observation: LLVM 14.0.6 produced a 1,668-byte typed-pointer fixture with bitcode magic
  `42 43 c0 de` and SHA-256
  `b45e3b74a3881b178c3d45310cc74d0bed3ece46e7101e6b9ac98a66aa301f01`.
  Evidence: llvmlite 0.42.0 `parse_assembly`/`as_bitcode`, followed by successful CUDA 12.2
  libNVVM verify/compile and `ptxas -arch=sm_75`.

- Observation: exploratory, non-retained LLVM 16 and NVRTC controls suggested that typed pointers
  alone do not make arbitrary modern LLVM output backward-readable and that NVRTC LTO output is a
  different container. These controls narrowed the investigation, but are not support evidence or
  an acceptance gate because their complete inputs and commands were not retained. The
  reproducible LLVM 14 fixture is the positive compatibility evidence.

- Observation: the existing LLVM 21 module is structurally the wrong producer, not merely missing
  one output method. It uses opaque pointers, the stock configuration does not request NVPTX or
  expose BitWriter/bitcode output, and its builder API has no address-space or named-metadata
  operations.
  Evidence: `ILLVMBuilder`, `LLVMBuilder`, the stock LLVM build scripts, and DLL exports.

Add evidence here as the probes run. Move settled conclusions to the Decision Log and design
document.

## Decision Log

- Decision: represent binary NVVM IR as `ObjectCode + LLVMIR + Kernel`.
  Rationale: `Assembly` versus `ObjectCode` is Slang's existing textual/binary distinction for the
  same payload. Exact descriptor comparisons preserve that distinction without inventing a public
  target or accepting host/library shapes.
  Date/Author: 2026-08-25, Codex.
  Revisit when: a public NVVM IR target needs stable file-extension or serialization behavior.

- Decision: do not infer binary input from the LLVM bitcode magic and do not append a terminator.
  Rationale: the artifact descriptor owns the representation; libNVVM owns format validation, and
  the byte count must preserve embedded NULs exactly.
  Date/Author: 2026-08-25, Codex.
  Revisit when: never unless the external API changes its buffer contract.

- Decision: treat a known LLVM 7-14 typed-pointer producer as a compatibility oracle, not an
  automatic production dependency.
  Rationale: the experiment must separate format feasibility from the larger dependency and
  builder-API decision.
  Date/Author: 2026-08-25, Codex.
  Revisit when: the probe identifies a small component that can be maintained and distributed with
  Slang.

- Decision: prototype a separate optional LLVM 14 module as the next NVVM builder implementation
  boundary.
  Rationale: NVIDIA's sample documents LLVM 7-14, the LLVM 14 fixture is accepted locally, and LLVM
  14 has first-class typed pointers. Keeping it separate avoids weakening the LLVM 21 CPU emitter or
  embedding a second incompatible LLVM in `slang-compiler`. A custom writer for a pinned NVVM
  subset remains possible, but the projected type, instruction, attribute, and metadata coverage
  makes a mature LLVM writer the lower-risk starting point.
  Date/Author: 2026-08-25, Codex.
  Revisit when: packaging measurements show the optional module is untenable or a maintained
  serializer demonstrates materially lower cost with equivalent verification coverage.

- Decision: embed the accepted fixture as a generated C++ byte array and retain its readable source
  and provenance beside the unit tests.
  Rationale: the real test remains independent of working directory and installed LLVM tools, while
  the fixture stays reproducible and reviewable.
  Date/Author: 2026-08-25, Codex.
  Revisit when: the test harness gains a hermetic copied-binary data mechanism.

## Outcomes and Retrospective

Slice 2 is complete. The binary consumer gate is positive and reuse of the existing LLVM 21
producer is negative. llvmlite 0.42.0's LLVM 14.0.6 writer produced the accepted 1,668-byte fixture
recorded above. CUDA 12.2 libNVVM verified and compiled it for `compute_75`; CUDA 12.2 `ptxas`
12.2.140 assembled the resulting `sm_75` PTX and reported four registers, no stack frame, and no
spills.

`NVVMDownstreamCompiler` now accepts exact `ObjectCode + LLVMIR + Kernel` bitcode as well as the
bootstrap assembly descriptor. Its shared blob path forwards binary bytes and embedded NULs
unchanged. The fixture source and provenance are under `tools/slang-unit-test/test-data/nvvm`, and
the byte array is compiled into the unit-test module so the test does not depend on a working
directory or local LLVM installation.

Validation on 2026-08-25 used the Debug build and produced:

- `slang-unit-test-tool/nvvm`: 18/18 passed, including real text and bitcode compilation plus
  offline `ptxas` assembly of the bitcode path;
- `slang-unit-test-tool/getDownstreamCompilerVersion`: 1/1 passed;
- `tests/downstream/downstream-compiler-version`: 7/7 passed; and
- `tests/cuda/sampler-comparison-state-unused`: 2/2 passed, preserving ordinary NVRTC PTX routing.

The new-helper/special-case inventory contains one artifact creator in tests, one manual fixture
generator, and two exact source descriptor constants in the external compiler boundary. The binary
shape is canonical because `ObjectCode` is Slang's established binary counterpart to `Assembly`;
the full descriptor prevents host or library LLVM artifacts from leaking into the kernel path. No
production code sniffs magic, rebuilds a Slang AST/IR representation, or patches malformed input.
The fake test fails if exact bytes or size are changed, and the real test delegates semantic
validation to libNVVM.

The next prerequisite is a dedicated optional LLVM 14 NVVM builder prototype. It must expose typed
pointers with address spaces, NVVM named metadata, and bitcode output without linking LLVM 14 into
`slang-compiler`. It must also prove that LLVM 14 and the LLVM 21 CPU module can coexist safely:
configure/build them separately, statically link only the required LLVM 14 components, hide LLVM
symbols, and export only a versioned Slang ABI. General Slang IR lowering and differential kernels
wait until that producer boundary exists.

The prototype package directory and its temporary PTX/cubin outputs were removed. The two real
bitcode tests passed again after cleanup, proving that the checked-in test path is hermetic. A
manual `generate.py` beside the source records the exact bitcode-to-header operation; it is not run
by the build and requires the pinned llvmlite tool only when deliberately regenerating the fixture.

## Context and Current Pipeline

Slice 0+1 added `NVVMDownstreamCompiler`. Its `compile` method accepts one
`Assembly + LLVMIR + Kernel` artifact, calls `loadBlob`, and passes that blob unchanged to
`nvvmAddModuleToProgram`. libNVVM accepts either text or bitcode through the same API, then verifies
and compiles the program to PTX. The established path remains:

```text
Slang IR -> CUDA C++ -> NVRTC -> PTX
```

The experimental external boundary currently proves:

```text
handwritten textual NVVM IR -> NVVMDownstreamCompiler -> libNVVM -> PTX
```

This slice adds only the binary half of that boundary:

```text
known typed-pointer writer -> LLVM bitcode fixture
                            -> NVVMDownstreamCompiler -> libNVVM -> PTX -> ptxas
```

The current CPU-oriented LLVM emitter is not yet in this path. It is backed by LLVM 21, uses opaque
pointers, lacks NVVM address spaces and annotations, and exposes no bitcode output method.

## Scope and Non-Goals

In scope:

- local CUDA 12.2/libNVVM 2.0 compatibility evidence;
- one minimal empty-kernel NVVM IR 2.0 bitcode fixture with reproducible provenance;
- an exact binary LLVM IR artifact input contract;
- byte-preserving fake-ABI coverage and real libNVVM/PTX/`ptxas` coverage;
- a structural audit of the LLVM 21 producer boundary without distorting production APIs;
- selection of the next writer implementation boundary.

Not in scope:

- Slang IR-to-NVVM lowering;
- address spaces, intrinsics, kernel parameters, libdevice, or runtime execution;
- CUDA emission-method routing or changes to `-target ptx`;
- making LLVM IR a public source language or compile target;
- shipping Python, llvmlite, `llvm-as`, or a downloaded LLVM package as a dependency;
- CUDA 13/Blackwell modern-dialect support or the final CI matrix.

## Architecture and Invariants

The source artifact kind is the representation discriminator:

- `Assembly + LLVMIR + Kernel` means textual NVVM IR;
- `ObjectCode + LLVMIR + Kernel` means LLVM bitcode.

The downstream compiler accepts only those two exact descriptors, exactly one source, and no
libraries in this slice. It does not sniff bytes, convert text to bitcode, or normalize a binary
payload. `loadBlob` returns owned bytes; their pointer and exact size flow unchanged to
`nvvmAddModuleToProgram`.

The checked-in fixture is test evidence, not a second IR model. Its adjacent source/provenance must
identify the writer version and generation command. No production code depends on fixture bytes.

## Interfaces and Dependencies

Expected production change:

- `source/compiler-core/slang-nvvm-compiler.cpp`: widen the exact input descriptor check to accept
  `ObjectCode + LLVMIR + Kernel`; no libNVVM ABI or public enum changes.

Expected tests:

- `tools/slang-unit-test/unit-test-nvvm-compiler.cpp`: fake byte-forwarding and descriptor rejection
  coverage, plus a real-toolkit bitcode compile;
- a small LLVM assembly source and generated byte include or binary fixture under a focused test
  data directory, with provenance recorded beside it.

Prototype-only tooling may use an isolated generated directory under `build/`; it must not become a
runtime/build dependency. The compatibility contract comes from NVIDIA's CUDA 12.2 libNVVM API and
NVVM IR 2.0 specification.

## Milestones

### Milestone 1: Establish writer evidence

Hypothesis: LLVM 14 with typed pointers emits a bitcode representation accepted by CUDA 12.2
libNVVM for the already-proven empty NVVM IR 2.0 kernel.

Use the same minimal module as Slice 0+1. Produce bitcode with an explicitly versioned writer in an
ignored build directory. Confirm the `BC C0 DE` wrapper, record its size and SHA-256, feed it to the
real libNVVM API, and assemble the returned PTX for `sm_75`.

Promotion criteria: verify, compile, and `ptxas` all succeed; the producer is versioned and can be
reproduced. Discard criteria: libNVVM rejects the bitcode or the producer silently uses opaque
pointers. If discarded, retain only diagnostics and try the next narrower typed-pointer producer.

### Milestone 2: Measure the current LLVM boundary

Determine whether LLVM 21 can serialize the same minimal module without adding a misleading
production API. If it can, submit the bytes to libNVVM and record the result. A failure is useful:
it proves a dedicated versioned writer is required. Do not weaken the pre-Blackwell contract or
reinterpret LLVM 21 text as compatible bitcode to manufacture a positive result.

Outcome: the shipped LLVM 21 module exposes no bitcode operation and cannot represent the required
typed function pointers, address spaces, or named metadata through `ILLVMBuilder`. Its stock build
does not request NVPTX or expose BitWriter/bitcode output. A runtime LLVM 21 writer probe would
therefore require adding an API solely to exercise a producer whose opaque-pointer representation
already violates the pre-Blackwell contract. The structural audit is sufficient to reject reuse;
the LLVM 14 fixture supplies the positive writer evidence.

### Milestone 3: Add the binary artifact contract

Accept the exact `ObjectCode + LLVMIR + Kernel` descriptor alongside the exact existing assembly
descriptor. Keep all loading, diagnostics, option construction, verification, compilation, and PTX
handling shared.

Add an always-running fake test with arbitrary binary bytes containing embedded NULs. Assert exact
pointer payload contents and size at `nvvmAddModuleToProgram`, successful PTX, diagnostics, and
program destruction. Add at least one near-miss descriptor rejection.

### Milestone 4: Preserve the real compatibility proof

Check in the minimal source plus a reproducible fixture representation. The real-toolkit test must
skip only when libNVVM is unavailable; a discovered incompatible library is a failure. Assert the
PTX contains `.visible .entry testEmpty`. Reuse or extend the unique-temp-path `ptxas` smoke so no
generated PTX/cubin remains in the repository.

### Milestone 5: Select the next writer implementation boundary

Compare these choices against the evidence:

1. a dedicated optional LLVM 7-14 NVVM builder module;
2. a version-pinned extension of the existing `ILLVMBuilder` distribution; or
3. a maintained narrow bitcode serializer independent of LLVM.

Choose only the next implementation boundary. Do not build general lowering in this slice.

Outcome: choose option 1 with LLVM 14. It is inside NVIDIA's documented LLVM 7-14 sample range and
the accepted fixture proves its typed-pointer output. Option 2 cannot preserve the LLVM 21 CPU
module and the LLVM 7 contract simultaneously. Option 3 could own only a pinned NVVM subset, but
its required type, instruction, attribute, and metadata coverage is not yet known; it remains a
fallback if the LLVM 14 module's measured packaging or maintenance cost is untenable.

### Milestone 6: Validate and audit

Build affected Debug targets; run the NVVM prefix, compiler-version tests, and the existing NVRTC
PTX regression. Run `git diff --check`, confirm generated artifacts were cleaned, and inventory each
new helper/special case under the `AGENTS.md` input-shape audit.

## Validation and Acceptance

Required local evidence:

- versioned writer output begins with LLVM bitcode magic and has a recorded SHA-256;
- CUDA 12.2 libNVVM verifies and compiles that fixture for `compute_75`;
- CUDA 12.2 `ptxas` accepts the resulting `sm_75` PTX;
- fake ABI test proves exact binary byte/count forwarding, including an embedded NUL;
- near-miss artifact descriptors fail before a libNVVM program is created;
- every existing NVVM test still passes;
- `slang-unit-test-tool/getDownstreamCompilerVersion`,
  `tests/downstream/downstream-compiler-version`, and
  `tests/cuda/sampler-comparison-state-unused` remain green;
- ordinary PTX routing remains NVRTC.

## Failure and Recovery

All prototype dependencies and generated outputs live under `build/` and may be regenerated. Do
not overwrite the Slice 0+1 textual fixture or alter ordinary PTX routing. If no candidate writer
passes, revert any unproven binary artifact change, preserve the negative evidence in this plan and
the design document, and make the next slice a dedicated-writer prototype.

If a real toolkit is absent, fake ABI tests still run but this compatibility slice cannot be marked
complete on that machine. If a candidate library is found but cannot load or rejects the known
fixture, fail with diagnostics rather than treating it as unavailable.

## Artifacts and Hand-Off

Retain in the repository only:

- this completed but uncommitted ExecPlan as a local hand-off log;
- durable conclusions in `docs/design/nvvm-backend.md`;
- the minimal textual source and its reproducible bitcode fixture if accepted;
- focused tests and the exact binary artifact contract; and
- the next writer-integration decision.

Do not retain downloaded packages, temporary PTX/cubin, probe executables, or ad hoc logs. Summarize
the motivating example, exact producer-to-consumer trace, rejected alternatives, and input-shape
audit in the eventual five-part PR description.
